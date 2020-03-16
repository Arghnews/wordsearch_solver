#include <algorithm>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <iterator>
#include <string>
#include <optional>
#include <type_traits>
#include <utility>
#include <set>
#include <vector>
#include <string_view>

#include <prettyprint.hpp>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include "trie.hpp"

using namespace std::literals;

int main()
{
  using TrieType = trie::Trie;

  struct ContainsAndFurtherResult
  {
    std::vector<std::size_t> contains;
    std::vector<std::size_t> further;
    std::vector<std::size_t> contains_and_further;
  };

  auto make_contains_and_further =
    [] (const TrieType& t,
        const std::string stem,
        const std::string suffixes)
    {
      ContainsAndFurtherResult result;
      t.contains_and_further(stem, suffixes,
          std::back_inserter(result.contains),
          std::back_inserter(result.further),
          std::back_inserter(result.contains_and_further));
      return result;
    };

  {
    std::vector<std::string> v{"hi", "there", "chum"};
    std::set<std::string> s{"hi", "there", "chum"};
    const auto const_v = v;
    const auto const_s = s;

    TrieType{};

    TrieType{v};
    TrieType{const_v};
    TrieType{std::vector{v}};

    TrieType{s};
    TrieType{const_s};
    TrieType{std::set{s}};

    TrieType{"a"};
    TrieType{"a"s};
    TrieType{"a"sv};

    TrieType{"a", "b"};
    TrieType{"a"s, "b"s};
    TrieType{"a"sv, "b"sv};

    TrieType{v.begin(), v.end()};
    TrieType{const_v.begin(), const_v.end()};
    TrieType{s.begin(), s.end()};
    TrieType{const_s.begin(), const_s.end()};

    TrieType{"a", "b", "a"};
    TrieType{"a"s, "b"s, "a"s};
    TrieType{"a"sv, "b"sv, "a"sv};
  }

  {
    const TrieType t{
      "a",
      "act",
      "acted",
      "actor",
      "actors",
    };
  }

  {
    TrieType t{"lapland", "laplanc"};
    // t.insert("lapland");
    // t.insert("laplanc");

    // (!t.contains("lap"));
    // (!t.contains("laplan"));
    // (t.contains("lapland"));
    // fmt::print("DONE\n");
    // return 0;

    fmt::print("\nTrie: {}End of trie\n", t);
    assert(t.size() == 2);
    assert(!t.contains("lap"));
    assert(!t.contains("laplan"));
    assert(t.contains("lapland"));
    assert(!t.contains("laplanda"));
    assert(!t.contains("laplandb"));
    assert(!t.contains("laplandab"));
    const auto word = "lapland"s;
    const bool found = t.contains(word);
    if (found)
    {
      fmt::print("Found {}\n", word);
    } else
    {
      fmt::print("Not found {}\n", word);
    }

    fmt::print("Done with lapland search test\n");
  }

  {
    const std::initializer_list<std::string> inserts{
      "act",
      "acted",
      "acting",
      "action",
      "actions",
      "actor",
      "activate",
    };
    TrieType t{inserts};
    assert(t.size() == inserts.size());

    // fmt::print("Traversing\n");
    // std::vector<std::string> traverse_words;
    // t.traverse([&traverse_words] (const auto& word)
        // {
          // fmt::print("{}\n", word);
          // traverse_words.push_back(word);
        // });
    // assert(std::is_sorted(traverse_words.begin(), traverse_words.end()));
    // assert(std::is_permutation(
          // traverse_words.begin(), traverse_words.end(),
          // inserts.begin(), inserts.end()
          // ));

    const std::string haystack = "activates";
    std::string w;
    for (const auto c: haystack)
    {
      w.push_back(c);
      bool has = t.contains(w);
      fmt::print("{}\n", t);
      bool has_prefix = t.further(w);
      fmt::print("Word: {:{}}, trie has {}: has prefix:{}\n", w,
          haystack.size(), has ? u8"✓ " : u8"❌", has_prefix ? u8"✓ " : u8"❌");
    }
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
    fmt::print("Trie: {}\n", t);
    for (const auto [substring, contains_answer, contains_prefix_answer]: v)
    {
      fmt::print("Izzard: {} {} {}\n", substring, contains_answer, contains_prefix_answer);
      fmt::print("Contains:\n");
      assert(t.contains(substring) == contains_answer);
      fmt::print("Further:\n");
      assert(t.further(substring) == contains_prefix_answer);
          // ,"substring: {}, expected: {} {}", substring, contains_answer,
          // contains_prefix_answer);
    }
  }

  {
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
    const TrieType t{bla};
    // fmt::print("{}\n", t.as_vector());

    auto bla2 = bla;
    std::sort(bla2.begin(), bla2.end());
    bla2.erase(std::unique(bla2.begin(), bla2.end()), bla2.end());
    fmt::print("Trie: {}\n", t);
    fmt::print("{}, size of input: {}\n", bla2, bla2.size());
    fmt::print("t.size: {}\n", t.size());
    assert(t.size() == bla2.size());
        // , "{} vs {}", t.size(), bla2.size());
    // assert(t.as_vector() == bla2);
    assert(std::all_of(bla.begin(), bla.end(),
          [&t] (const auto& str)
          {
            fmt::print("{}\n", t);
            return t.contains(str);
          }));
  }

  {
    const std::set<std::string> w{"hi", "there", "chum"};
    const TrieType t{w.begin(), w.end()};
    fmt::print("Trie: {}\n", t);
    // const auto v = t.as_vector();
    // assert(std::equal(w.begin(), w.end(), v.begin(), v.end()));
    assert(!t.further("y"));
    assert(!t.further("yq"));
  }

  {
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
    const TrieType t{bla};

    auto bla2 = bla;
    std::sort(bla2.begin(), bla2.end());
    bla2.erase(std::unique(bla2.begin(), bla2.end()), bla2.end());
    fmt::print("{}\n", bla2);
    assert(t.size() == bla2.size());
    assert(t.size() == bla2.size());
        //, "{} vs {}", t.size(), bla2.size());
    assert(std::all_of(bla.begin(), bla.end(),
          [&t] (const auto& str)
          {
            return t.contains(str);
          }));

    assert(!t.contains("burne"));
    assert(!t.contains("buzn"));
    assert(!t.contains("ba"));
    assert(!t.contains("zooz"));
    assert(!t.contains("x"));
    for (const auto c: "abcdefghijklmnopqrstuvwxyz"sv)
    {
      fmt::print("{}\n", t);
      assert(!t.contains(std::string{"ahem"} + c));
    }
    assert(t.further("bu"));
    assert(t.further("aheaa"));
    assert(t.further("ahe"));
    assert(t.further("a"));
    assert(t.further("b"));
    assert(t.further("z"));
    assert(t.further("zo"));
    assert(!t.further("burning"));
    assert(!t.further("boomer"));

    const auto result = make_contains_and_further(t, "burn", "aei");
    assert(result.contains.size() == 0);
    assert(result.further.size() == 2);
    assert(result.further.at(0) == 1);
    assert(result.further.at(1) == 2);
    assert(result.contains_and_further.size() == 0);

  }

  {
    fmt::print("----------------\n");
    const std::vector<std::string> bla{
      "ahem",
      "ahe",
      "aheaaa",
      "ahem",
    };
    fmt::print("Inserting {}\n", bla);
    const TrieType t{bla};
    fmt::print("Trie: {}\n", t);

    {
      const auto result = make_contains_and_further(t, "ahe", "amz");
      fmt::print("Contains: {}\n", result.contains);
      fmt::print("Further: {}\n", result.further);
      fmt::print("Contains and Further: {}\n", result.contains_and_further);
      assert(result.contains.size() == 1 && result.contains.at(0) == 1);
      assert(result.further.size() == 1);
      assert(result.contains_and_further.size() == 0);
    }

    {
      const auto result = make_contains_and_further(t, "", "ab");
    }
  }

  {
    const std::set<std::string> w{"hi", "there", "chum"};
    TrieType t_orig{w.begin(), w.end()};
    fmt::print("Move cons\n");
    const auto t{std::move(t_orig)};
    // const auto v = t.as_vector();
    // assert(std::equal(w.begin(), w.end(), v.begin(), v.end()));
    assert(!t.further("y"));
    assert(!t.further("yq"));
  }

  {
    fmt::print("Start of wadoogey\n");
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
    TrieType t{w};
    for (auto word: w)
    {
      assert(t.contains(word));
    }
    assert(!w.empty());
    for (auto i = 0ULL; i + 1 < w.size(); ++i)
    {
      assert(t.further(w.at(i)));
    }
    assert(!t.further(w.back()));
    assert(t.size() == 9);
    fmt::print("End of wadoogey\n");
  }

  {
    TrieType t{};
    assert(!t.contains("asdf"));
    assert(!t.further("asdf"));
    assert(!t.contains("a"));
    assert(!t.further("a"));

    const auto result = make_contains_and_further(t, "", "");
    assert(result.contains.empty());
    assert(result.further.empty());
    assert(result.contains_and_further.empty());
  }

  {
    const TrieType t{};
    assert(!t.contains(""));
    assert(!t.further(""));
  }

  {
    fmt::print("\nMaking empty trie:\n");
    const TrieType t{""};
    fmt::print("Empty trie: {}\n", t);
    assert(t.contains(""));
    assert(!t.further(""));
    fmt::print("Trie with empty string as only elem: {}\n", t);
    assert(t.size() == 1);
  }

  {
    const TrieType t{"", "a", "b", "c"};
    assert(t.contains(""));
    assert(t.further(""));
    assert(t.contains("a"));
    assert(t.contains("b"));
    assert(t.contains("c"));
    assert(t.size() == 4);

    {
      const auto result = make_contains_and_further(t, "", "");
      assert(result.contains.empty());
      assert(result.further.empty());
      assert(result.contains_and_further.empty());
    }
    {
      const auto result = make_contains_and_further(t, "", "abcd");
      assert(result.contains.size() == 3);
      assert(result.contains.at(0) == 0);
      assert(result.contains.at(1) == 1);
      assert(result.contains.at(2) == 2);

      assert(result.further.size() == 0);
      assert(result.contains_and_further.empty());
    }
  }

  {
    const TrieType t{"aaa", "aab", "aabx", "aaby", "aabz", "aabxa", "aabxb",
      "aabyc", "aabyd", "aac"};
    fmt::print("Trie: {}\n", t);
    assert(!t.further("aaa"));
    assert(!t.further("aac"));
    assert(t.further("aabx"));
    assert(t.further("aaby"));
    assert(!t.further("aabz"));
    assert(!t.further("aabyc"));
    assert(!t.further("aabycd"));
  }
}
