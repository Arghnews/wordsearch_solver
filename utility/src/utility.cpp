#include "wordsearch_solver/utility/utility.hpp"

#include <fmt/core.h>
#include <range/v3/action/remove_if.hpp>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

void close_file(std::FILE* fp) {
  if (fp)
    std::fclose(fp);
}

// We use C style internals, fopen requires a null terminated string
// std::string_view does not guarantee null termination
template <class ConsumeFile>
auto read_file_wrapper(const char* filepath, ConsumeFile&& f) {
  // https://en.cppreference.com/w/cpp/io/c/fgetc

  std::unique_ptr<std::FILE, decltype(&close_file)> fp(
      std::fopen(filepath, "r"), &close_file);
  if (!fp) {
    throw std::runtime_error(fmt::format("Cannot open file {}", filepath));
  }

  auto val = f(fp);

  if (std::ferror(fp.get())) {
    throw std::runtime_error("I/O error when reading");
  } else if (!std::feof(fp.get())) {
    throw std::runtime_error("End of file not reached");
  }

  return val;
}

} // namespace

std::vector<std::string> utility::read_file_as_lines(const char* filepath) {
  auto ff = [](const auto& fp) {
    std::vector<std::string> lines;
    std::string line;

    int c; // note: int, not char, required to handle EOF
    while ((c = std::fgetc(fp.get())) != EOF) {
      if (c == '\n') {
        lines.push_back(std::move(line));
        line.clear();
      } else {
        line.push_back(static_cast<char>(c));
      }
    }

    return lines;
  };
  return read_file_wrapper(filepath, ff);
}

std::vector<std::string>
utility::read_file_as_lines(const std::string& filepath) {
  return read_file_as_lines(filepath.data());
}

std::string read_file_as_string(const char* filepath) {
  auto ff = [](const auto& fp) {
    std::string file_contents;
    int c; // note: int, not char, required to handle EOF
    while ((c = std::fgetc(fp.get())) != EOF) {
      file_contents.push_back(static_cast<char>(c));
    }
    return file_contents;
  };
  return read_file_wrapper(filepath, ff);
}

std::string read_file_as_string(const std::string& filepath) {
  return read_file_as_string(filepath.data());
}

std::vector<std::string>
utility::read_file_as_lines_keep_lowercase_ascii_only(const char* filepath) {
  return read_file_as_lines(filepath) |
         ranges::actions::remove_if([](const auto& word) {
           for (const auto c : word) {
             if (!(c >= 97 && c < 123)) {
               return true;
             }
           }
           return false;
         });
}

std::vector<std::string> utility::read_file_as_lines_keep_lowercase_ascii_only(
    const std::string& filepath) {
  return read_file_as_lines_keep_lowercase_ascii_only(filepath.data());
}
