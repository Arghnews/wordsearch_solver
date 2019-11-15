#include <algorithm>
#include <bits/c++config.h> // Remove this, coc.nvim suggests it
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <ios>
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "prettyprint.hpp"
#include "wadoo.h"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include "jr_assert.h"

using namespace std::literals;

namespace detail
{

struct TrieImpl
{
  std::vector<TrieImpl*> children_;
  char value_;
  bool is_end_of_word;

  TrieImpl() = default;
  explicit TrieImpl(char c)
    : children_()
    , value_(c)
    , is_end_of_word(false)
  {}

  ~TrieImpl()
  {
    for (const auto child: children_)
    {
      delete child;
    }
  }

  TrieImpl* insert(char c)
  {
    JR_ASSERT(c != '\0', "Do not want to insert null");
    auto it = children_.begin();
    for (; it != children_.end(); ++it)
    {
      if ((*it)->value_ == c)
      {
        return *it;
      } else if ((*it)->value_ > c) // Ascii specific, case specific
      {
        break;
      }
    }
    return *children_.insert(it, new TrieImpl(c));
  }

  TrieImpl* lookup(char c) const
  {
    for (const auto child: children_)
    {
      if (child->value_ == c)
      {
        return child;
      }
    }
    return nullptr;
  }

};

// Whether trie has words that start_with prefix, excluding the prefix word
bool contains_prefix(const TrieImpl& t, const std::string& prefix)
{
  JR_ASSERT(!prefix.empty(), "Empty word lookup?");

  auto it = prefix.begin();
  const auto* tp = &t;

  if (prefix.begin() == prefix.end())
  {
    return true;
  }
  for (;; ++it)
  {
    if (it == prefix.end())
    {
      if (tp != nullptr && !tp->children_.empty())
      {
        return true;
      }
      return false;
    } else if (tp == nullptr)
    {
      return false;
    }
    tp = tp->lookup(*it);
  }
  JR_ASSERT(false, "Unreachable");
}

bool contains(const TrieImpl& t, const std::string& word)
{
  auto it = word.begin();
  const auto* tp = &t;

  JR_ASSERT(!word.empty(), "Empty word lookup?");
  if (word.begin() == word.end())
  {
    return true;
  }
  for (;; ++it)
  {
    if (it == word.end())
    {
      if (tp != nullptr && tp->is_end_of_word == true)
      {
        return true;
      }
      return false;
    } else if (tp == nullptr)
    {
      return false;
    }
    tp = tp->lookup(*it);
  }
  JR_ASSERT(false, "Unreachable");
}

std::pair<TrieImpl*, bool> insert(TrieImpl& t, const std::string& word)
{
  auto* tp = &t;
  for (const char c: word)
  {
    // fmt::print("Inserting {}\n", c);
    tp = tp->insert(c);
  }
  if (!tp->is_end_of_word)
  {
    tp->is_end_of_word = true;
    return {tp, true};
  }
  return {tp, false};
}

template<class F>
void traverse(const TrieImpl& root, F&& f)
{
  std::vector<std::pair<const TrieImpl*, decltype(TrieImpl::children_)::const_iterator>> v;
  std::string stack;
  // Assume root is single elemed
  // JR_ASSERT(root.children_.size() == 1);
  // v.emplace_back(root.children_.front(), root.children_.front()->children_.begin());
  if (!root.children_.empty())
  {
    v.emplace_back(&root, root.children_.begin());
  }

  while (!v.empty())
  {
    // fmt::print("\n--\n");
    const auto& [t, t_child_it] = v.back();
    if (t_child_it == t->children_.begin())
    {
      if (t->value_ != '\0') // special case for root, to remove
      {
        // fmt::print("Adding value: {}\n", t->value_);
        stack.push_back(t->value_);
      }
      if (t->is_end_of_word)
      {
        // Coroutine anyone?
        f(stack);
        // fmt::print("Output: {}\n", stack);
      }
    }
    if (t->children_.end() != t_child_it)
    {
      v.emplace_back(*t_child_it, (*t_child_it)->children_.begin());
      // Iterators may be invalidated so must get position afresh and not reuse
      // previous references to iterators
      // Loop only runs while !v.empty() and we just emplace_back so end -2 ok
      ++std::prev(v.end(), 2)->second;
    } else
    {
      while (!v.empty() && v.back().second == v.back().first->children_.end())
      {
        v.pop_back();
        stack.pop_back();
      }
    }

  }

}

} // namespace detail

class Trie
{
  public:
  bool contains(const std::string& key) const
  {
    return detail::contains(trie_, key);
  }
  bool contains_prefix(const std::string& key) const
  {
    return detail::contains_prefix(trie_, key);
  }
  bool insert(const std::string& word)
  {
    // If make pretty iterators can return iterator to inserted here
    const auto [tp, inserted] = detail::insert(trie_, word);
    size_ += inserted;
    return inserted;
  }
  template<class Container>
  void copy_insert_all(const Container& c)
  {
    for (const auto& val: c)
    {
      this->insert(val);
    }
  }
  std::vector<std::string> as_vector() const
  {
    std::vector<std::string> traverse_words;
    this->traverse([&traverse_words] (const auto& word)
        {
          fmt::print("{}\n", word);
          traverse_words.push_back(word);
        });
    return traverse_words;
  }
  std::size_t size() const
  {
    return size_;
  }
  Trie() = default;
  explicit Trie(const std::initializer_list<std::string>& keys)
    : trie_()
    , size_()
  {
    this->copy_insert_all(keys);
  }
  explicit Trie(const std::vector<std::string>& keys)
    : trie_()
    , size_()
  {
    this->copy_insert_all(keys);
  }
  template<class F>
  void traverse(F&& f) const
  {
    detail::traverse(trie_, f);
  }
  private:
  detail::TrieImpl trie_;
  std::size_t size_;
};

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
      "zoo",
      "burn",
      "burning",
      "burned",
      "burner",
      "burner",
    };
    Trie t{};
    t.copy_insert_all(bla);
    fmt::print("{}\n", t.as_vector());

    auto bla2 = bla;
    std::sort(bla2.begin(), bla2.end());
    bla2.erase(std::unique(bla2.begin(), bla2.end()), bla2.end());
    fmt::print("{}\n", bla2);
    JR_ASSERT(t.size() == bla2.size(), "{} vs {}", t.size(), bla2.size());
    JR_ASSERT(t.as_vector() == bla2);
  }

}
