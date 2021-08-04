// Wordsearch solver gui
// Uses ImGui

// #define GLFW_INCLUDE_NONE // GLFW including OpenGL headers causes ambiguity
// or Dear ImGui: standalone example application for GLFW + OpenGL 3, using
// programmable pipeline (GLFW is a cross-platform general purpose library for
// handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation,
// etc.) If you are new to Dear ImGui, read documentation from the docs/ folder
// + read the top of imgui.cpp. Read online:
// https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <glad/glad.h> // Initialize with gladLoadGL()

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <wordsearch_solver/wordsearch_solver.hpp>

#include <gperftools/profiler.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

template <class T, class E>
std::optional<typename T::size_type> find_index(const T &container,
                                                const E &elem) {
  const auto it = std::find(container.begin(), container.end(), elem);
  if (it == container.end()) {
    return {};
  }
  // Apparently don't need cast from difference_type to size_type
  return {std::distance(container.begin(), it)};
}

class UserWordsRegex {
  std::string user_words_regex_string_ = "";
  std::string last_reg_ = "";
  std::optional<std::regex> user_words_regex_ = {};
  bool changed_ = false;

public:
  UserWordsRegex() = default;

  // To be used with ImGui::InputText to write data into
  std::string *get_string_addr() { return &user_words_regex_string_; }

  const std::string &get_string() const { return user_words_regex_string_; }

  void reset() { *this->get_string_addr() = ""; }

  const std::optional<std::regex> &get() {
    if (user_words_regex_string_ != last_reg_) {
      last_reg_ = user_words_regex_string_;
      changed_ = true;
      try {
        // fmt::print("Recompiled reg\n");
        user_words_regex_.emplace(user_words_regex_string_,
                                  std::regex::optimize);
      } catch (const std::regex_error &) {
        // fmt::print("Failed recompile reg\n");
        user_words_regex_ = {};
      }
    }
    return user_words_regex_;
  }

  bool changed() const { return changed_; }
  void reset_changed() { changed_ = false; }
  bool if_changed_reset() {
    if (this->changed()) {
      this->reset_changed();
      return true;
    }
    return false;
  }
};

struct CellData {
  bool selected_event = false;
  bool user_selected = false;
  bool word_selected = false;
  bool is_selected() const { return user_selected || word_selected; }
};

class SortState {
  bool lexi_ = false;
  bool by_size_ = false;
  // Having a bool for both lexi and by_size is actually superfluous, need only
  // one since now defaulting to lexi
  bool reversed_ = false;
  bool dirty_ = false;

public:
  void sort_lexicographically() {
    lexi_ = true;
    by_size_ = false;
    reversed_ = false;
    dirty_ = true;
  }

  void sort_by_size() {
    lexi_ = false;
    by_size_ = true;
    reversed_ = false;
    dirty_ = true;
  }

  void reverse() {
    reversed_ = !reversed_;
    dirty_ = true;
  }

  bool if_dirty_reset() {
    assert(!(lexi_ && by_size_));
    if (dirty_) {
      dirty_ = false;
      return true;
    }
    return false;
  }

  std::function<bool(const std::string &, const std::string &)>
  get_comparator() const {
    if (by_size_) {
      if (!reversed_) {
        return [](const auto &word0, const auto &word1) {
          // Reverse sort so longest first,
          // sort secondarily lexicographically so sort is stable
          return std::forward_as_tuple(word1.size(), word0) <
                 std::forward_as_tuple(word0.size(), word1);
        };
      } else {
        return [](const auto &word0, const auto &word1) {
          return std::forward_as_tuple(word0.size(), word0) <
                 std::forward_as_tuple(word1.size(), word1);
        };
      }
    } else {
      // Default to lexi
      if (reversed_) {
        return std::greater<void>{};
      } else {
        return std::less<void>{};
      }
    }
  }
};

class SelectedWord {
public:
  struct IndexPosition {
    solver::Index index;
    std::size_t position;
  };

private:
  std::string word_;
  solver::Tail indexes_;
  std::vector<IndexPosition> index_positions_in_word_;

public:
  SelectedWord(const std::string &word, const solver::Tail &indexes,
               const std::vector<IndexPosition> &index_positions_in_word)
      : word_(word), indexes_(indexes),
        index_positions_in_word_(index_positions_in_word) {}

  const std::string &word() const { return word_; }
  const solver::Tail &indexes() const { return indexes_; }
  using PositionsIt = std::vector<IndexPosition>::const_iterator;

  std::pair<PositionsIt, PositionsIt> index_positions_in_word() const {
    return {index_positions_in_word_.begin(), index_positions_in_word_.end()};
  }
};

class McData {
  using Words = std::vector<std::string>;
  Words dictionary_;
  solver::WordsearchGrid wordsearch_grid_;
  trie::Trie trie_;
  solver::WordToListOfListsOfIndexes results_;
  std::map<std::string, std::size_t> word_indexes_selected_;
  std::optional<SelectedWord> selected_word_;

public:
  std::vector<CellData> cell_data;
  Words result_words;
  Words result_words_filtered;
  UserWordsRegex user_words_regex = {};
  SortState sort_state = {};

  const solver::WordsearchGrid &wordsearch_grid() const {
    return wordsearch_grid_;
  }

  solver::WordsearchGrid &wordsearch_grid() { return wordsearch_grid_; }

  CellData &cell_at(const solver::Index &index) {
    return cell_data[index.y * wordsearch_grid_.columns() + index.x];
  }

  void unset_all_indexes_selected() {
    for (auto &p : cell_data) {
      p = {};
    }
  }

  auto get_index_positions_in_word() const {
    // Returns a begin/end pair of iterators into a sorted vector of
    // pair<solver::Index, std::size_t> index in wordsearch, position in word to
    // be used by colour/shading of letters
    return selected_word_
               ? std::optional{selected_word_.value().index_positions_in_word()}
               : std::nullopt;
  }

  void reset_selected_word() { selected_word_.reset(); }
  bool selected_word_has_value() const { return selected_word_.has_value(); }

  void set_selected_word(const std::string &word) {
    const auto &lists_of_indexes = results_.at(word);
    const auto index_selected = word_indexes_selected_.at(word);
    const auto &indexes = lists_of_indexes[index_selected];

    std::vector<SelectedWord::IndexPosition> index_positions_in_word;
    index_positions_in_word.reserve(word.size());
    for (const auto &index : indexes) {
      const auto position_in_word = find_index(indexes, index);
      assert(position_in_word);
      // // Not sure why emplace_back doesn't work here
      // index_positions_in_word.emplace_back(index, *position_in_word);
      index_positions_in_word.push_back(
          SelectedWord::IndexPosition{index, *position_in_word});
    }
    std::sort(
        index_positions_in_word.begin(), index_positions_in_word.end(),
        [](const auto &v0, const auto &v1) { return v0.index < v1.index; });

    selected_word_.emplace(word, indexes, std::move(index_positions_in_word));
  }

  std::size_t selected_word_selected_index() const {
    return word_indexes_selected_.at(this->get_selected_word());
  }

  std::size_t selected_word_total_indexes() const {
    return results_.at(this->get_selected_word()).size();
  }

  void increment_selected_word_indexes_selected() {
    assert(selected_word_);
    const auto word = selected_word_->word();
    ++word_indexes_selected_[word] %= results_.at(word).size();
    this->set_selected_word(word);
  }

  void decrement_selected_word_indexes_selected() {
    assert(selected_word_);
    const auto word = selected_word_->word();
    --word_indexes_selected_[word] %= results_.at(word).size();
    this->set_selected_word(word);
  }

  const std::string &get_selected_word() const {
    return selected_word_.value().word();
  }

  std::optional<std::reference_wrapper<const solver::Tail>>
  get_indexes_for_selected_word() const {
    return selected_word_ ? std::optional{std::cref(selected_word_->indexes())}
                          : std::nullopt;
  }

  McData(const Words &dict, const solver::WordsearchGrid &grid)
      : dictionary_(dict), wordsearch_grid_(grid), trie_(dict),
        results_(solver::solve(trie_, grid)) {

    for (auto row = 0UL; row < wordsearch_grid_.rows(); ++row) {
      for (auto column = 0UL; column < wordsearch_grid_.columns(); ++column) {
        cell_data.emplace_back();
      }
    }

    for (const auto &[word, _] : results_) {
      result_words.emplace_back(word);
      word_indexes_selected_[word] = 0;
    }

    result_words_filtered = result_words;
  }
};

int main(int, char **) {

  auto data = McData{
      utility::read_file_as_lines("dictionary.txt"),
      solver::make_grid(utility::read_file_as_lines("wordsearch.txt")),
  };

  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  // Decide GL+GLSL versions
  const char *glsl_version = "#version 400";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(
      1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
  if (window == NULL)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  bool err = gladLoadGL() == 0;
  if (err) {
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    return 1;
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
  // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsClassic();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

  // Our state
  bool show_demo_window = false;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  ProfilerRestartDisabled();
  ProfilerEnable();

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application. Generally you may always pass all inputs
    // to dear imgui, and hide them from your application based on those two
    // flags.
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in
    // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
    // ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    // My window
    {

      ImGui::Begin("Wordsearch solver!", nullptr,
                   ImGuiWindowFlags_AlwaysAutoResize);

      const std::size_t rows = data.wordsearch_grid().rows();
      const std::size_t cols = data.wordsearch_grid().columns();
      assert(rows > 0);
      assert(cols > 0);

      if (ImGui::Button("Clear all")) {
        data.unset_all_indexes_selected();
        data.user_words_regex.reset();
        data.reset_selected_word();
      }

      {
        // Display wordsearch letters in a grid as a table

        const auto indexes = data.get_indexes_for_selected_word();
        auto index_positions_in_word = data.get_index_positions_in_word();

        // const auto index_positions_it = data

        if (ImGui::BeginTable("table1", static_cast<int>(cols))) {
          for (auto row = 0UL; row < rows; row++) {
            ImGui::TableNextRow();
            for (auto column = 0UL; column < cols; column++) {
              ImGui::TableSetColumnIndex(static_cast<int>(column));
              const std::string s{data.wordsearch_grid()(row, column)};
              const auto index = solver::Index{row, column};

              ImGui::PushID(static_cast<int>(row * cols + column));

              std::optional<std::size_t> position_in_word = {};

              if (index_positions_in_word) {
                auto &[first, last] = *index_positions_in_word;
                // assert(first != last);
                // Keep first pointing at the next index position pair
                // If the current index equals this index, then we're at this
                // position in the word. This can be used for shading it
                if (first != last && first->index == index) {
                  position_in_word = first->position;
                  assert(first < last);
                  ++first;
                }
              }

              if (position_in_word) {
                // Want to highlight the letters of the selected word, with
                // highest intensity for the first letter, trailing off till the
                // last quick and dirty way to do so

                // Scale the value to between 0.2f and 0.8f so that the
                // highlighting isn't too bright or too dark
                const auto v =
                    0.2f +
                    (0.6f *
                     (1 - (static_cast<float>(*position_in_word) /
                           static_cast<float>((indexes->get()).size()))));
                // I don't understand floating point math well enough to know if
                // this is necessary/correct, but adding tiny delta in case
                assert(v >= 0.19f);
                assert(v <= 0.81f);

                // ImGui::PushStyleColor(ImGuiCol_FrameBg,
                ImGui::PushStyleColor(ImGuiCol_Header,
                                      (ImVec4)ImColor::HSV(0.1f, 0.9f, v));
              }

              auto &cell = data.cell_at(index);
              cell.word_selected = position_in_word.has_value();

              const bool selected = cell.is_selected();
              if (ImGui::Selectable(s.data(), selected)) {
                // fmt::print("Event on {}\n", index);
                cell.selected_event = true;
              }

              if (position_in_word.has_value()) {
                ImGui::PopStyleColor();
              }
              ImGui::PopID();
            }
          }

          ImGui::EndTable();
        }
      }

      // Process any selection/deselection events
      // NOTE: for now do nothing with these
      for (auto &cell : data.cell_data) {
        if (cell.selected_event) {
          if (!cell.is_selected()) {
            // fmt::print("Selected {}\n", index);
            cell.user_selected = true;
          } else {
            // fmt::print("Deselected {}\n", index);
            cell.user_selected = false;
          }
          cell.selected_event = false;
        }
      }

      {
        // User regex input line

        ImGui::InputText("", data.user_words_regex.get_string_addr());
        ImGui::SameLine();
        if (data.user_words_regex.get_string().empty() ||
            data.user_words_regex.get()) {
          const auto total = fmt::format("{}", data.result_words.size());
          const auto matches = fmt::format(
              "{:>{}}", data.result_words_filtered.size(), total.size());
          ImGui::Text("%s/%s matches", matches.data(), total.data());
        } else {
          // Regex string invalid
          ImGui::Text("Invalid regular expression");
        }
      }

      {
        // GUI sorting buttons

        if (ImGui::Button("Sort alphabetically")) {
          data.sort_state.sort_lexicographically();
        }
        ImGui::SameLine();
        if (ImGui::Button("Sort by length")) {
          data.sort_state.sort_by_size();
        }
        ImGui::SameLine();
        if (ImGui::Button("Reverse order")) {
          data.sort_state.reverse();
        }
      }

      {
        // Update word lists depending on if the regex has changed, or a sort
        // has been clicked

        // Here we store our selection data as an index.
        const auto regex_empty = data.user_words_regex.get_string().empty();
        const auto &re = data.user_words_regex.get();

        bool filter_dirty = false;
        if (data.sort_state.if_dirty_reset()) {
          std::sort(data.result_words.begin(), data.result_words.end(),
                    data.sort_state.get_comparator());
          filter_dirty = true;
        }

        if (filter_dirty || data.user_words_regex.if_changed_reset()) {
          if (regex_empty) {
            data.result_words_filtered = data.result_words;
          } else if (!re) {
            data.result_words_filtered.clear();
          } else if (!regex_empty && re) {
            data.result_words_filtered.clear();
            std::copy_if(data.result_words.begin(), data.result_words.end(),
                         std::back_inserter(data.result_words_filtered),
                         [&re](const auto &word) {
                           return std::regex_search(word, *re);
                         });
          }
        }
      }

      {
        // Arrow buttons to scroll through indexes for the selected word

        if (data.selected_word_has_value()) {
          // const auto &word = data.get_selected_word();
          // const auto total_indexes = lists_of_indexes.size();
          // auto &index_selected = data.word_indexes_selected[word];
          const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;

          ImGui::PushButtonRepeat(true);
          if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
            data.decrement_selected_word_indexes_selected();
          }
          ImGui::SameLine(0.0f, spacing);
          if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
            data.increment_selected_word_indexes_selected();
          }
          ImGui::PopButtonRepeat();
          ImGui::SameLine();
          ImGui::Text("Word selected: %s", data.get_selected_word().data());

          ImGui::Text("%lu / %lu", data.selected_word_selected_index() + 1,
                      data.selected_word_total_indexes());

        } else {
          ImGui::Text("No word selected");
        }
      }

      {
        // Scrolling output of words that passed the regex filter, sorted
        // according to the sorts

        int processed = 0;
        if (ImGui::BeginListBox("listbox 1")) {

          // Clipper is ImGui's neat way to only display a small portion of the
          // results that are shown to the user, rather than the entire list
          ImGuiListClipper clipper;
          clipper.Begin(static_cast<int>(data.result_words_filtered.size()));
          while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
              const auto &s =
                  data.result_words_filtered[static_cast<std::size_t>(i)];
              const bool selected = data.selected_word_has_value() &&
                                    s == data.get_selected_word();
              if (ImGui::Selectable(s.data(), selected)) {
                // fmt::print("Selected {}\n", s);
                // data.selected_word = s;
                data.set_selected_word(s);
                ImGui::SetItemDefaultFocus();
              }
              ++processed;
            }
          }

          ImGui::EndListBox();
        }

        ImGui::Text("Processed %d", processed);
      }

      ImGui::Text("Filtered output words/total: %lu/%lu",
                  data.result_words_filtered.size(), data.result_words.size());
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  ProfilerStop();

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
