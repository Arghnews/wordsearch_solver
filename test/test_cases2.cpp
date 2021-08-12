#include "test_cases.hpp"

#include "wordsearch_solver/wordsearch_solver.hpp"

#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <range/v3/action/sort.hpp>
#include <range/v3/action/unique.hpp>
#include <range/v3/view/map.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// TODO:
// Currently test fails because of unsorted thing.
// Could try to create some kind of CRTP wrapper that classes implement/inherit
// from to implement the contains/further stuff. So they all get the constructor
// wrappers for free. Also find cmake way to run tests for one. Find cmake
// restructure to have all targets in top level cmakelists and pass that to
// solver with variable (easy) Fork prettyprint.hpp repo and add nice stuff for
// optional configure, and cmake support Nicer way to pass all classes to run
// through from build to program would be awesome really. Need to copy SFINAE
// for string_view -> string construction from set to vector

// To make adding a dictionary implementation less painful, add it to this
// "list", delimited by a comma
// #define WORDSEARCH_DICTIONARY_CLASSES compact_trie::CompactTrie, trie::Trie,
// \
  // compact_trie2::CompactTrie2, dictionary_std_vector::DictionaryStdVector, \
  // dictionary_std_set::DictionaryStdSet

using namespace std::literals;

namespace fs = std::filesystem;
using ContainsFurther = std::pair<bool, bool>;

template <class Dict>
std::vector<std::pair<bool, bool>> static make_contains_and_further(
    const Dict& t, const std::string& stem, const std::string& suffixes) {
  std::vector<std::pair<bool, bool>> contains_further;
  t.contains_further(stem, suffixes, std::back_inserter(contains_further));
  return contains_further;
}

TEMPLATE_TEST_CASE("Test trie constructors", "[construct]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  std::vector<std::string> v{"hi", "there", "chum"};
  std::set<std::string> s{"hi", "there", "chum"};
  const auto const_v = v;
  const auto const_s = s;

  TestType{};

  TestType{v};
  TestType{const_v};
  TestType{std::vector{v}};

  TestType{s};
  TestType{const_s};
  TestType{std::set{s}};

  TestType{"a"};
  TestType{"a"s};
  TestType{"a"sv};

  TestType{"a", "b"};
  TestType{"a"s, "b"s};
  TestType{"a"sv, "b"sv};

  TestType{v.begin(), v.end()};
  TestType{const_v.begin(), const_v.end()};
  TestType{s.begin(), s.end()};
  TestType{const_s.begin(), const_s.end()};

  TestType{"a", "b", "a"};
  TestType{"a"s, "b"s, "a"s};
  TestType{"a"sv, "b"sv, "a"sv};

  const TestType t{
      "a", "act", "acted", "actor", "actors",
  };
}

TEMPLATE_TEST_CASE("Simple contains", "[contains]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  TestType t{"lapland", "laplanc"};
  // t.insert("lapland");
  // t.insert("laplanc");

  // (!t.contains("lap"));
  // (!t.contains("laplan"));
  // (t.contains("lapland"));
  // INFO(fmt::format("DONE\n"));
  // return 0;

  INFO(fmt::format("\nTrie: {}\n", t));
  CHECK(t.size() == 2);
  CHECK(!t.contains("lap"));
  CHECK(!t.contains("laplan"));
  CHECK(t.contains("lapland"));
  CHECK(!t.contains("laplanda"));
  CHECK(!t.contains("laplandb"));
  CHECK(!t.contains("laplandab"));
  const auto word = "lapland"s;
  const bool found = t.contains(word);
  if (found) {
    INFO(fmt::format("Found {}\n", word));
  } else {
    INFO(fmt::format("Not found {}\n", word));
  }

  INFO(fmt::format("Done with lapland search test\n"));
}

TEMPLATE_TEST_CASE("Contains and further test", "[contains][further]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  // clang-format off
  const std::initializer_list<std::string> inserts{
    "act",
    "acted",
    "acting",
    "action",
    "actions",
    "actor",
    "activate",
  };
  // clang-format on
  TestType t{inserts};
  CHECK(t.size() == inserts.size());

  // INFO(fmt::format("Traversing\n"));
  // std::vector<std::string> traverse_words;
  // t.traverse([&traverse_words] (const auto& word)
  // {
  // INFO(fmt::format("{}\n", word));
  // traverse_words.push_back(word);
  // });
  // CHECK(std::is_sorted(traverse_words.begin(), traverse_words.end()));
  // CHECK(std::is_permutation(
  // traverse_words.begin(), traverse_words.end(),
  // inserts.begin(), inserts.end()
  // ));

  const std::string haystack = "activates";
  std::string w;
  for (const auto c : haystack) {
    w.push_back(c);
    bool has = t.contains(w);
    INFO(fmt::format("{}\n", t));
    bool has_prefix = t.further(w);
    INFO(fmt::format("Word: {:{}}, trie has {}: has prefix:{}\n", w,
                     haystack.size(), has ? u8"✓ " : u8"❌",
                     has_prefix ? u8"✓ " : u8"❌"));
  }
  // clang-format off
  const std::vector<std::tuple<std::string, bool, bool>> v{
    { "a",         false, true, },
    { "ac",        false, true, },
    { "act",       true,  true, },
    { "acti",      false, true, },
    { "activ",     false, true, },
    { "activa",    false, true, },
    { "activat",   false, true, },
    { "activate",  true,  false, },
    { "activates", false, false, },
  };
  // clang-format on
  INFO(fmt::format("Trie: {}\n", t));
  for (const auto& [substring, contains_answer, contains_prefix_answer] : v) {
    INFO(fmt::format("Izzard: {} {} {}\n", substring, contains_answer,
                     contains_prefix_answer));
    INFO(fmt::format("Contains:\n"));
    CHECK(t.contains(substring) == contains_answer);
    INFO(fmt::format("Further:\n"));
    CHECK(t.further(substring) == contains_prefix_answer);
    // ,"substring: {}, expected: {} {}", substring, contains_answer,
    // contains_prefix_answer);
  }
}

TEMPLATE_TEST_CASE("Contains and further test 2", "[contains][further]",
                   WORDSEARCH_DICTIONARY_CLASSES) {

  // clang-format off
  const std::vector<std::string> bla{
    "burner",
    "zoo",
    "zoological",
    "ahem",
    "ahe",
    "ahem",
    "boom",
    "boomer",
    "zoo",
    "burn",
    "burning",
    "burned",
    "burner",
    "burner",
  };
  // clang-format on
  const TestType t{bla};
  // INFO(fmt::format("{}\n", t.as_vector()));

  auto bla2 = bla;
  std::sort(bla2.begin(), bla2.end());
  bla2.erase(std::unique(bla2.begin(), bla2.end()), bla2.end());
  INFO(fmt::format("{}\n", bla2));
  CHECK(t.size() == bla2.size());
  // , "{} vs {}", t.size(), bla2.size());
  // CHECK(t.as_vector() == bla2);
  for (const auto& word : bla) {
    CHECK(t.contains(word));
    INFO(fmt::format("{} should be in {}\n", word, t));
  }
}

TEMPLATE_TEST_CASE("Simple further", "[further]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  const std::set<std::string> w{"hi", "there", "chum"};
  const TestType t{w.begin(), w.end()};
  INFO(fmt::format("Trie: {}\n", t));
  // const auto v = t.as_vector();
  // CHECK(std::equal(w.begin(), w.end(), v.begin(), v.end()));
  CHECK(!t.further("y"));
  CHECK(!t.further("yq"));
}

TEMPLATE_TEST_CASE("More complex contains further",
                   "[contains][further][contains_further]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  // clang-format off
  const std::vector<std::string> bla{
    "burner",
    "zoo",
    "zoological",
    "ahem",
    "ahe",
    "aheaaaaaaaaa",
    "ahem",
    "boom",
    "boomer",
    "zoo",
    "burn",
    "burning",
    "burned",
    "burner",
    "burner",
    "bur",
  };
  // clang-format on
  const TestType t{bla};

  auto bla2 = bla;
  std::sort(bla2.begin(), bla2.end());
  bla2.erase(std::unique(bla2.begin(), bla2.end()), bla2.end());
  INFO(fmt::format("{}\n", bla2));
  CHECK(t.size() == bla2.size());
  for (const auto& word : bla) {
    CHECK(t.contains(word));
    INFO(fmt::format("{} should be in {}\n", word, t));
  }

  CHECK(!t.contains("burne"));
  CHECK(!t.contains("buzn"));
  CHECK(!t.contains("ba"));
  CHECK(!t.contains("zooz"));
  CHECK(!t.contains("x"));
  for (const auto c : "abcdefghijklmnopqrstuvwxyz"sv) {
    INFO(fmt::format("{}\n", t));
    CHECK(!t.contains(std::string{"ahem"} + c));
  }
  CHECK(t.further("bu"));
  CHECK(t.further("aheaa"));
  CHECK(t.further("ahe"));
  CHECK(t.further("a"));
  CHECK(t.further("b"));
  CHECK(t.further("z"));
  CHECK(t.further("zo"));
  CHECK(!t.further("burning"));
  CHECK(!t.further("boomer"));

  const auto result = make_contains_and_further(t, "burn", "aei");
  CHECK(result.size() == 3);
  CHECK(result.at(0) == ContainsFurther(false, false));
  CHECK(result.at(1) == ContainsFurther(false, true));
  CHECK(result.at(2) == ContainsFurther(false, true));
}

TEMPLATE_TEST_CASE("Contains_further test", "[contains_further]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  INFO(fmt::format("----------------\n"));
  // clang-format off
  const std::vector<std::string> bla{
    "ahem",
    "ahe",
    "aheaaa",
    "ahem",
  };
  // clang-format on
  INFO(fmt::format("Inserting {}\n", bla));
  const TestType t{bla};
  INFO(fmt::format("Trie: {}\n", t));

  {
    const auto result = make_contains_and_further(t, "ahe", "amz");
    CHECK(result.size() == 3);
    CHECK(result.at(0) == ContainsFurther(false, true));
    CHECK(result.at(1) == ContainsFurther(true, false));
    CHECK(result.at(2) == ContainsFurther(false, false));
    INFO(fmt::format("ContainsFurther: {}\n", result));
  }

  {
    const auto result = make_contains_and_further(t, "", "ab");
    CHECK(result.size() == 2);
    CHECK(result.at(0) == ContainsFurther(false, true));
    CHECK(result.at(1) == ContainsFurther(false, false));
  }
}

TEMPLATE_TEST_CASE("Test move cons", "[construct][further]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  const std::set<std::string> w{"hi", "there", "chum"};
  TestType t_orig{w.begin(), w.end()};
  INFO(fmt::format("Move cons\n"));
  const auto t{std::move(t_orig)};
  // const auto v = t.as_vector();
  // CHECK(std::equal(w.begin(), w.end(), v.begin(), v.end()));
  CHECK(!t.further("y"));
  CHECK(!t.further("yq"));
}

TEMPLATE_TEST_CASE("Contains and further test 3", "[contains][further]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  INFO(fmt::format("Start of wadoogey\n"));
  // clang-format off
  std::vector<std::string> w =
  {
    "a"        ,
    "ac"       ,
    "act"      ,
    "acti"     ,
    "activ"    ,
    "activa"   ,
    "activat"  ,
    "activate" ,
    "activates",
  };
  // clang-format on
  TestType t{w};
  for (auto word : w) {
    CHECK(t.contains(word));
  }
  CHECK(!w.empty());
  for (auto i = 0UL; i + 1 < w.size(); ++i) {
    CHECK(t.further(w.at(i)));
  }
  CHECK(!t.further(w.back()));
  CHECK(t.size() == 9);
  INFO(fmt::format("End of wadoogey\n"));
}

TEMPLATE_TEST_CASE("Empty dict/trie tests", "[contains][further][construct]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  {
    TestType t{};
    CHECK(!t.contains("asdf"));
    CHECK(!t.further("asdf"));
    CHECK(!t.contains("a"));
    CHECK(!t.further("a"));

    const auto result = make_contains_and_further(t, "", "");
    CHECK(result.empty());
  }

  {
    const TestType t{};
    CHECK(!t.contains(""));
    CHECK(!t.further(""));
  }

  {
    INFO(fmt::format("\nMaking empty trie:\n"));
    const TestType t{""};
    INFO(fmt::format("Empty trie: {}\n", t));
    CHECK(t.contains(""));
    CHECK(!t.further(""));
    INFO(fmt::format("Trie with empty string as only elem: {}\n", t));
    CHECK(t.size() == 1);
  }

  {
    const TestType t{"", "a", "b", "c"};
    CHECK(t.contains(""));
    CHECK(t.further(""));
    CHECK(t.contains("a"));
    CHECK(t.contains("b"));
    CHECK(t.contains("c"));
    CHECK(t.size() == 4);

    {
      const auto result = make_contains_and_further(t, "", "");
      CHECK(result.empty());
    }
    {
      const auto result = make_contains_and_further(t, "", "abcd");

      CHECK(result.size() == 4);
      CHECK(result.at(0) == ContainsFurther(true, false));
      CHECK(result.at(1) == ContainsFurther(true, false));
      CHECK(result.at(2) == ContainsFurther(true, false));
      CHECK(result.at(3) == ContainsFurther(false, false));
    }
  }
}

TEMPLATE_TEST_CASE("Further tests", "[further]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  const TestType t{"aaa",   "aab",   "aabx",  "aaby",  "aabz",
                   "aabxa", "aabxb", "aabyc", "aabyd", "aac"};
  INFO(fmt::format("Trie: {}\n", t));
  CHECK(!t.further("aaa"));
  CHECK(!t.further("aac"));
  CHECK(t.further("aabx"));
  CHECK(t.further("aaby"));
  CHECK(!t.further("aabz"));
  CHECK(!t.further("aabyc"));
  CHECK(!t.further("aabycd"));
}

TEMPLATE_TEST_CASE("Printable", "[printable]", WORDSEARCH_DICTIONARY_CLASSES) {
  const TestType t{};
  fmt::format("{}", t);
  std::stringstream ss;
  ss << t;
}

TEMPLATE_TEST_CASE("Empty and size functions", "[empty][size]",
                   WORDSEARCH_DICTIONARY_CLASSES) {
  {
    TestType t{};
    CHECK(t.empty());
    CHECK(t.size() == 0);
  }
  {
    TestType t{"hi"};
    CHECK(!t.empty());
    CHECK(t.size() == 1);
  }
  {
    TestType t{"hi", "hi"};
    CHECK(!t.empty());
    CHECK(t.size() == 1);
  }
  {
    TestType t{"a", "b"};
    CHECK(!t.empty());
    CHECK(t.size() == 2);
  }
}
