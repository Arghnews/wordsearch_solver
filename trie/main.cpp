#include <algorithm>
#include <bits/c++config.h> // Remove this, coc.nvim suggests it
#include <cassert>
#include <cstdint>
#include <future>
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

#include "prettyprint.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include "jr_assert/jr_assert.h"
#include "trie.h"

using namespace std::literals;

template<typename T, class>
void toCustomString(std::vector<T>) {
    // general case
}

// Forgot that you can specialise function templates but unlike class templates
// you can't partially specialise them, that's the difference
template<>
void toCustomString<std::string, int>(std::vector<std::string>) {
    // strings
}

template<class First, class Last, class ValueType = typename First::value_type>
struct Permutations
{
  using value_type = typename First::value_type;
  Permutations(First first, Last last,
      value_type lim, value_type init = value_type{})
    : first(first)
    , last(last)
    , init(init)
    , lim(lim)
  {
  }
  Permutations(First first, Last last)
    : Permutations(first, last,
        static_cast<std::size_t>(std::distance(first, last)))
  {}

  template<class Element>
  static First gen_next(First top, const Last last,
      const Element& init,
      const Element& lim)
  {
    for (top = std::find(top, last, lim); top != last;)
    {
      if (*top == lim)
      {
        *top++ = init;
      }
      else
      {
        ++(*top);
        if (*top != lim)
        {
          break;
        }
      }
    }
    return top;
  }

  bool operator()() const
  {
    // const auto first = indexes.begin();
    // const auto last = indexes.end();
    if (first == last) return false;
    // const auto lim = static_cast<std::size_t>(std::distance(first, last));
    if (*first < lim)
    {
      ++*first;
    }
    if (*first == lim)
    {
      return gen_next(first, last, init, lim) != last;
    }
    return true;

    // for (auto top = first; top != last;)
    // {
      // for (; *first < lim; ++*first)
      // {
        // // OUTPUT indexes
        // fmt::print("{}\n", std::vector<std::size_t>{first, last});
      // }
      // top = gen_next(first, last, lim);
    // }
  }

  First first;
  Last last;
  value_type init;
  value_type lim;

};

template<class TrieType>
static int tests();

int main()
{
  // std::vector<std::size_t> n{0, 1, 2, 3, 4};
  // fmt::print("1 number combos\n");
  // for (auto i = 0ULL; i < n.size(); ++i)
  // {
    // fmt::print("{}\n", i);
  // }
  // fmt::print("2 number combos\n");


  // for (auto i = 0ULL; i < n.size(); ++i)
  // {
    // for (auto j = i; j < n.size(); ++j)
    // {
      // fmt::print("{}, {}\n", i, j);
    // }
  // }
  // fmt::print("3 number combos\n");

  // auto gen = [](auto...){};

  using namespace trie;
  const std::vector<std::string> bla{
    // "burner",
    // "zoo",
    // "zoological",
    // "ahem",
    "ahe",
    // "aheaaaaaaaaa",
    // "ahem",
    // "boom",
    // "boomer",
    // "zoo",
    // "burn",
    // "burning",
    // "burned",
    // "burner",
    // "burner",
    // "bur",
  };

  const Trie t{bla};
  CompactTrieImpl ct{t};

  tests<Trie>();
  tests<CompactTrie>();

  // for (const auto& word: bla)
  // {
    // for (auto i = 1ULL; i < 8; ++i)
    // {
      // const auto initial = 'a';
      // std::string indexes(i, initial);
      // Permutations perms{indexes.rbegin(), indexes.rend()
        // , 'z' + 1, initial
        // };
      // do
      // {
        // // fmt::print("Tada!\n");
        // // auto indexes = "a";
        // const auto trie_contains = t.contains(indexes);
        // const auto ctrie_contains = ct.contains(indexes);
        // const auto trie_further = t.further(indexes);
        // const auto ctrie_further = ct.further(indexes);

        // fmt::print("{}\n", indexes);
        // JR_ASSERT(trie_contains == ctrie_contains,
            // "Word: \"{}\". trie_contains: {} vs ctrie_contains: {}\n"
            // "CompactTrieImpl: {}\n"
            // "Dict: {}",
            // indexes, trie_contains, ctrie_contains,
            // ct, t.as_vector());
        // JR_ASSERT(t.further(indexes) == ct.further(indexes),
            // "Word: {}. trie_further: {} vs ctrie_further: {}",
            // indexes, trie_further, ctrie_further);
      // } while (perms());
    // }
    // fmt::print("Donezo!\n");
  // }

  /*
  [0, 1, 2], 0, 0
  [0, 1, 2], 1, 0
  [0, 1, 2], 2, 0
  2, 2, 0
  [0, 1, 2], 0, 1
  [0, 1, 2], 1, 1
  [0, 1, 2], 2, 1
  2, 2, 1
  [0, 1, 2], 0, 2
  [[0, 1, 2]], [0, 1, 2], 2
  */

  {
    auto n = 3ULL;
    for (auto i = 0ULL; i < n; ++i)
    {
      for (auto j = i; j < n; ++j)
      {
        for (auto k = j; k < n; ++k)
        {
          fmt::print("{}, {}, {}\n", i, j, k);
        }
      }
    }
  }

  return 0;

  {
    std::string s{"abcd"};
    // for ()
    {
    }

  }

  {
    std::string s{"abcde"};
    do {
      fmt::print("{}\n", s);
    } while (std::next_permutation(s.begin(), s.end()));
  }

  using namespace trie;

  std::vector<std::string> v{"hi", "there", "chum"};
  std::set<std::string> s{"hi", "there", "chum"};
  const auto const_v = v;
  const auto const_s = s;

  Trie{};

  Trie{v};
  Trie{const_v};
  Trie{std::vector{v}};

  Trie{s};
  Trie{const_s};
  Trie{std::set{s}};

  Trie{"a"};
  Trie{"a"s};
  Trie{"a"sv};
  // Trie{'a'};

  Trie{"a", "b"};
  Trie{"a"s, "b"s};
  Trie{"a"sv, "b"sv};
  // Trie{'a', 'b'};

  Trie{v.begin(), v.end()};
  Trie{const_v.begin(), const_v.end()};
  Trie{s.begin(), s.end()};
  Trie{const_s.begin(), const_s.end()};

  Trie{"a", "b", "a"};
  Trie{"a"s, "b"s, "a"s};
  Trie{"a"sv, "b"sv, "a"sv};

  // {
    // Trie t{};
    // Trie::T c{t};
    // auto res = c.search("a");
    // JR_ASSERT(res == c.end());
  // }

  // {
    // Trie t{"a"};
    // Trie::T c{t};
    // auto res = c.search("a");
    // fmt::print("{}\n", c);
    // JR_ASSERT(res == c.compressed_.begin());
  // }

  // {
    // Trie t{"a", "aa"};
    // Trie::T c{t};
    // auto res = c.search("a");
    // fmt::print("{}\n", c);
    // JR_ASSERT(res == c.compressed_.begin());
  // }

  // {
    // fmt::print("\n---------------------\n\n");
    // Trie t{"a", "ab"};
    // JR_ASSERT(t.size() == 2);
    // const Trie::T c{t};
    // auto res = c.search("ab");
    // fmt::print("{} at {}\n", c, std::distance(c.compressed_.begin(), res));
    // // JR_ASSERT(res == c.end());
    // JR_ASSERT(res == std::next(c.compressed_.begin(), 1));
  // }

  // {
    // fmt::print("\n---------------------\n\n");
    // Trie t{"a", "ab", "abc"};
    // JR_ASSERT(t.size() == 3);
    // Trie::T c{t};

    // {
      // auto res = c.search("abc");
      // fmt::print("{} at {}\n", c, std::distance(c.compressed_.cbegin(), res));
      // JR_ASSERT(res == std::next(c.compressed_.cbegin(), 2));
    // }
    // {
      // auto res = c.search("ab");
      // fmt::print("{} at {}\n", c, std::distance(c.compressed_.cbegin(), res));
      // JR_ASSERT(res == std::next(c.compressed_.cbegin(), 1));
    // }
    // {
      // auto res = c.search("a");
      // fmt::print("{} at {}\n", c, std::distance(c.compressed_.cbegin(), res));
      // JR_ASSERT(res == std::next(c.compressed_.cbegin(), 0));
    // }
    // {
      // auto res = c.search("aba");
      // fmt::print("{} at {}\n", c, std::distance(c.compressed_.cbegin(), res));
      // JR_ASSERT(res == c.end());
    // }
    // {
      // auto res = c.search("abaaaa");
      // fmt::print("{} at {}\n", c, std::distance(c.compressed_.cbegin(), res));
      // JR_ASSERT(res == c.end());
    // }

}

template<class TrieType>
static int tests()
{
  using namespace trie;

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
    TrieType t{};
    t.insert("lapland");
    t.insert("laplanc");

    // (!t.contains("lap"));
    // (!t.contains("laplan"));
    // (t.contains("lapland"));
    // fmt::print("DONE\n");
    // return 0;

    fmt::print("Trie: {}\n", t);
    JR_ASSERT(t.size() == 2);
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
    for (const auto [substring, contains_answer, contains_prefix_answer]: v)
    {
      JR_ASSERT(t.contains(substring) == contains_answer);
      JR_ASSERT(t.further(substring) == contains_prefix_answer,
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
    const TrieType t{bla};
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
            fmt::print("{}\n", t);
            return t.contains(str);
          }));
  }

  {
    const std::set<std::string> w{"hi", "there", "chum"};
    const TrieType t{w.begin(), w.end()};
    const auto v = t.as_vector();
    JR_ASSERT(std::equal(w.begin(), w.end(), v.begin(), v.end()));
  }

  {
    const char* arr[]{"hi", "there", "chum"};
    const auto* const p = arr;
    const auto* const p2 = p + std::size(arr);
    const TrieType t{p, p2};
    const auto v = t.as_vector();
    fmt::print("As vector: {}\n", v);
    JR_ASSERT(std::is_permutation(
          std::begin(arr), std::end(arr), v.begin(), v.end()));
  }

  {
    const std::set<std::string> w{"hi", "there", "chum"};
    const TrieType t{w.begin(), w.end()};
    const auto v = t.as_vector();
    JR_ASSERT(std::equal(w.begin(), w.end(), v.begin(), v.end()));
    JR_ASSERT(!t.further("y"));
    JR_ASSERT(!t.further("yq"));
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
    fmt::print("TrieType t:{}\n", t.as_vector());

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
    for (const auto c: "abcdefghijklmnopqrstuvwxyz"sv)
    {
      fmt::print("{}\n", t);
      JR_ASSERT(!t.contains(std::string{"ahem"} + c));
    }

    auto str = [&bla] (auto&& s)
    {
      JR_ASSERT(std::find(bla.begin(), bla.end(), s) != bla.end(),
          "Error in test setup, {} not in vector", s);
      return s;
    };
    {
      struct
      {
        std::vector<std::size_t> contains;
        std::vector<std::size_t> further;
        std::vector<std::size_t> contains_and_further;
      } result;
      t.contains_and_further(str("ahe"), "amz",
          std::back_inserter(result.contains),
          std::back_inserter(result.further),
          std::back_inserter(result.contains_and_further)
          );
      fmt::print("{}\n", result.contains);
      fmt::print("{}\n", result.further);
      fmt::print("{}\n", result.contains_and_further);
      JR_ASSERT(result.contains.size() == 1 && result.contains.at(0) == 1);
      JR_ASSERT(result.further.size() == 1);
      JR_ASSERT(result.contains_and_further.size() == 0);
    }

  }

  {
    const std::set<std::string> w{"hi", "there", "chum"};
    TrieType t_orig{w.begin(), w.end()};
    fmt::print("Move cons\n");
    const auto t{std::move(t_orig)};
    const auto v = t.as_vector();
    JR_ASSERT(std::equal(w.begin(), w.end(), v.begin(), v.end()));
    JR_ASSERT(!t.further("y"));
    JR_ASSERT(!t.further("yq"));
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
      JR_ASSERT(t.contains(word));
    }
    JR_ASSERT(t.size() == 9);
    fmt::print("End of wadoogey\n");
  }

  fmt::print("Done, exiting main!\n");
  return 0;
}
