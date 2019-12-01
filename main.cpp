#include <algorithm>
#include <bits/c++config.h> // Remove this, coc.nvim suggests it
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

#include "prettyprint.hpp"
#include "trie.h"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include "jr_assert.h"

using namespace std::literals;

int main()
{

  {
    Trie t{};
    t.insert("lapland");

    JR_ASSERT(t.size() == 1);
    JR_ASSERT(!t.contains("lap"));
    JR_ASSERT(!t.contains("laplan"));
    JR_ASSERT(t.contains("lapland"));
    JR_ASSERT(!t.contains("laplanda"));
    const auto word = "lapland"s;
    const bool found = t.contains(word);
    if (found)
    {
      fmt::print("Found {}\n", word);
    } else
    {
      fmt::print("Not found {}\n", word);
    }
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
    Trie t{inserts};
    JR_ASSERT(t.size() == inserts.size());

    fmt::print("Traversing\n");
    std::vector<std::string> traverse_words;
    t.traverse([&traverse_words] (const auto& word)
        {
          fmt::print("{}\n", word);
          traverse_words.push_back(word);
        });
    JR_ASSERT(std::is_sorted(traverse_words.begin(), traverse_words.end()));
    JR_ASSERT(std::is_permutation(
          traverse_words.begin(), traverse_words.end(),
          inserts.begin(), inserts.end()
          ));

    const std::string haystack = "activates";
    std::string w;
    for (const auto c: haystack)
    {
      w.push_back(c);
      bool has = t.contains(w);
      bool has_prefix = t.contains_prefix(w);
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
    for (const auto [substring, contains_answer, contains_prefix_answer]: v)
    {
      JR_ASSERT(t.contains(substring) == contains_answer);
      JR_ASSERT(t.contains_prefix(substring) == contains_prefix_answer,
          "substring: {}, expected: {} {}", substring, contains_answer,
          contains_prefix_answer);
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
    const Trie t{bla};
    fmt::print("{}\n", t.as_vector());

    auto bla2 = bla;
    std::sort(bla2.begin(), bla2.end());
    bla2.erase(std::unique(bla2.begin(), bla2.end()), bla2.end());
    fmt::print("{}\n", bla2);
    JR_ASSERT(t.size() == bla2.size(), "{} vs {}", t.size(), bla2.size());
    JR_ASSERT(t.as_vector() == bla2);
    JR_ASSERT(std::all_of(bla.begin(), bla.end(),
          [&t] (const auto& str)
          {
            return t.contains(str);
          }));
  }

  {
    const std::set<std::string> w{"hi", "there", "chum"};
    const Trie t{w.begin(), w.end()};
    const auto v = t.as_vector();
    JR_ASSERT(std::equal(w.begin(), w.end(), v.begin(), v.end()));
  }

  {
    const char* arr[]{"hi", "there", "chum"};
    const auto* const p = arr;
    const auto* const p2 = p + std::size(arr);
    const Trie t{p, p2};
    const auto v = t.as_vector();
    fmt::print("As vector: {}\n", v);
    JR_ASSERT(std::is_permutation(
          std::begin(arr), std::end(arr), v.begin(), v.end()));
  }

  {
    const std::set<std::string> w{"hi", "there", "chum"};
    const Trie t{w.begin(), w.end()};
    const auto v = t.as_vector();
    JR_ASSERT(std::equal(w.begin(), w.end(), v.begin(), v.end()));
    JR_ASSERT(!t.contains_prefix("y"));
    JR_ASSERT(!t.contains_prefix("yq"));
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
    const Trie t{bla};
    fmt::print("{}\n", t.as_vector());

    auto bla2 = bla;
    std::sort(bla2.begin(), bla2.end());
    bla2.erase(std::unique(bla2.begin(), bla2.end()), bla2.end());
    fmt::print("{}\n", bla2);
    JR_ASSERT(t.size() == bla2.size(), "{} vs {}", t.size(), bla2.size());
    JR_ASSERT(t.as_vector() == bla2);
    JR_ASSERT(std::all_of(bla.begin(), bla.end(),
          [&t] (const auto& str)
          {
            return t.contains(str);
          }));

    JR_ASSERT(!t.contains("burne"));
    JR_ASSERT(!t.contains("buzn"));
    JR_ASSERT(!t.contains("ba"));
    JR_ASSERT(!t.contains("zooz"));
    JR_ASSERT(!t.contains("x"));
    for (const auto c: "abcdefghijklmnopqrstuvwxyz")
    {
      JR_ASSERT(!t.contains(std::string{"ahem"} + c));
    }

    auto str = [&bla] (auto&& s)
    {
      JR_ASSERT(std::find(bla.begin(), bla.end(), s) != bla.end(),
          "Error in test setup, {} not in vector", s);
      return s;
    };
    auto result = t.contains_and_further(str("ahe"), "amz");
    JR_ASSERT(result.contains.size() == 1 && result.contains.at(0) == 1);
    JR_ASSERT(result.further.size() == 1);
    JR_ASSERT(result.contains_and_further.size() == 0);

    auto result2 = t.contains_and_further("bur", "amzn");
    JR_ASSERT(result2.contains.size() == 0);
    JR_ASSERT(result2.further.size() == 0);
    JR_ASSERT(result2.contains_and_further.size() == 1);
    JR_ASSERT(result2.contains_and_further.at(0) == 3);
  }

}

