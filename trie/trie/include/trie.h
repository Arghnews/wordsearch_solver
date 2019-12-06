#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <prettyprint.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include "wordsearch_solver_defs.h"

// #include "jr_assert.h"

namespace trie
{

using ResultType = wordsearch_solver::Result;
// using ResultType = struct Result
// {
  // std::vector<std::size_t> contains;
  // std::vector<std::size_t> further;
  // std::vector<std::size_t> contains_and_further;
// };

namespace detail
{

struct TrieImpl
{
  std::vector<TrieImpl*> children_;
  char value_;
  bool is_end_of_word_;

  TrieImpl() = default;
  explicit TrieImpl(char c)
    : children_()
    , value_(c)
    , is_end_of_word_(false)
  {
  }

  TrieImpl(const TrieImpl&) = delete;
  TrieImpl& operator=(const TrieImpl&) = delete;

  TrieImpl(TrieImpl&& other) noexcept
  {
    std::cout << "Move cons trie " << __PRETTY_FUNCTION__ << "\n";
    *this = std::move(other);
  }

  TrieImpl& operator=(TrieImpl&& other) noexcept
  {
    std::cout << "Move assign op trie " << __PRETTY_FUNCTION__ << "\n";
    children_ = std::move(other.children_);
    other.children_.clear();
    value_ = other.value_;
    is_end_of_word_ = other.is_end_of_word_;
    return *this;
  }

  ~TrieImpl()
  {
    // std::cout << "TrieImpl destructor " << __PRETTY_FUNCTION__ <<
      // " on " << *this << "\n";
    for (const auto child: children_)
    {
      // std::cout << "Deleting child ->" << *child << "\n";
      delete child;
    }
  }

  friend std::ostream& operator<<(std::ostream& os, const TrieImpl& trie)
  {
    os << trie.value_ << " ";
    if (trie.is_end_of_word_) os << "end_of_word";
    os << "children: (" << trie.children_.size() << "): ";
    for (const auto child: trie.children_)
    {
      os << child->value_;
    }
    return os;
  }

  TrieImpl* insert(char c)
  {
    //JR_ASSERT(c != '\0', "Do not want to insert null");
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

// TODO:
// this class' move constructor due to tp_ pointing to a variable in the class
// that owns this doesn't work nicely. Fix/think about this
struct Cache
{
  const TrieImpl *tp_;
  mutable std::string cached_str_;
  mutable std::vector<const TrieImpl *> cached_ptrs_;
  mutable std::size_t hits = 0;
  mutable std::size_t misses = 0;

  Cache() = default;

  Cache(const TrieImpl* tp)
    : tp_(tp)
    , cached_str_()
    , cached_ptrs_()
  {}

  Cache(const Cache&) = delete;
  Cache(Cache&&) = default;

  Cache& operator=(const Cache&) = delete;
  Cache& operator=(Cache&&) = default;

  ~Cache()
  {
    std::cout << "Cache hits: " << hits << " vs misses: " << misses <<
      " -- hits per miss: " <<
      static_cast<double>(hits) / static_cast<double>(misses) << "" << "\n";
  }

  // inline
  std::pair<TrieImpl*, std::size_t>
  lookup(const TrieImpl* tp, const std::string& word) const
  {
    assert(tp);
    assert(tp_);
    // fmt::print("tp: {} vs tp_: {}\n", tp, &tp_);
    // fmt::print("tp: {}\n", *tp);
    // fmt::print("tp_: {}\n", *tp_);
    // fmt::print("tp_ vs tp, {} vs {}\n", tp_, tp);
    assert(tp_ == tp && "For now only supports lookup from root. "
        "May fail on move, let's see");
    std::size_t i = 0;
    {
      const auto& cache = cached_str_;
      assert(cache.size() == cache.size());
      // for (const auto c: cached_str)
      const auto shorter = std::min(cache.size(), word.size());
      // fmt::print("Shorter -> {}\n", shorter);
      // TODO: replace with std::equal?
      for (; i < shorter && cache[i] == word[i]; ++i)
      {
        // tp = cached_ptrs_[i];
        // fmt::print("Setting tp to {}\n", ppp(tp));
      }
      if (shorter > 0 && i > 0)
      {
        tp = cached_ptrs_[i - 1];
      }
      assert(cache.size() == cached_ptrs_.size());
      if (i != 0)
      {
        assert(tp);
        //fmt::print("Cached output: {}, {}\n", i, tp->value_);
      }
    }
    hits += i;
    misses += word.size() - i;
    cached_str_.resize(i);
    cached_ptrs_.resize(i);
    // fmt::print("End cache loop with i: {}\n", i);
    // TODO: Hmmmmmmmmmmmmmmmmmmmmmmm. Is this *the* solution?
    using T = decltype(tp);
    using TypeWithoutConst = std::conditional_t<std::is_pointer_v<T>,
          std::add_pointer_t<std::remove_const_t<std::remove_pointer_t<T>>>,
          T>;
    // auto a = const_cast<TT>(tp);
    // decltype(a)::name;
    return {const_cast<TypeWithoutConst>(tp), i};
  }

  void append(const TrieImpl* const tp) const
  {
    assert(tp);
    if (!tp->value_)
      fmt::print("THE VALUE IS NULL\n");
    // assert(tp->value_);
    cached_str_.push_back(tp->value_);
    cached_ptrs_.push_back(tp);
  }

};

inline bool is_end_of_word(const TrieImpl* tp)
{
  return tp && tp->is_end_of_word_;
}

inline bool has_children(const TrieImpl* tp)
{
  return tp && !tp->children_.empty();
}

template<class F>
inline void traverse(const TrieImpl& root, F&& f)
{
  std::vector<std::pair<
    const TrieImpl*, decltype(TrieImpl::children_)::const_iterator>> v;
  std::string stack;
  // Assume root is single elemed
  // //JR_ASSERT(root.children_.size() == 1);
  // v.emplace_back(
  // root.children_.front(), root.children_.front()->children_.begin());
  if (!root.children_.empty())
  {
    v.emplace_back(&root, root.children_.begin());
  }

  while (!v.empty())
  {
    // //fmt::print("\n--\n");
    const auto& [t, t_child_it] = v.back();
    if (t_child_it == t->children_.begin())
    {
      if (t->value_ != '\0') // special case for root, to remove
      {
        // //fmt::print("Adding value: {}\n", t->value_);
        stack.push_back(t->value_);
      }
      if (t->is_end_of_word_)
      {
        // Coroutine anyone?
        f(stack);
        // //fmt::print("Output: {}\n", stack);
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

template<class T>
static const void* v(const T* t)
{
  return static_cast<const void*>(t);
}

} // namespace detail

class Trie
{
  public:
  [[nodiscard]] bool contains(const std::string& key) const
  {
    return detail::is_end_of_word(this->lookup(key));
  }

  [[nodiscard]] bool further(const std::string& key) const
  {
    return detail::has_children(this->lookup(key));
  }

  bool insert(const std::string& word)
  {
    assert(!word.empty());

    auto [tp, i] = cache_.lookup(&trie_, word);
    // detail::TrieImpl* t = tp;
    for (; i < word.size(); ++i)
    {
      // tp = const_cast<detail::TrieImpl*>(tp)->insert(word.at(i));
      // const auto prev_tp = tp;
      // cache_.append(tp);
      tp = tp->insert(word[i]);
      cache_.append(tp);
      // cache_.append(prev_tp);
    }
    if (!tp->is_end_of_word_)
    {
      tp->is_end_of_word_ = true;
      ++size_;
      return true;
    }
    return false;
    // return nullptr;
  }

  void contains_and_further(const std::string& stem,
      const std::string& suffixes, ResultType& result) const
  {
    //fmt::print("contains_and_further called with stem: {} and suffixes: {}\n",
        // stem, suffixes);

    const auto* const tp = this->lookup(stem);
    for (auto i = 0ULL; i < suffixes.size(); ++i)
    {
      // stem.push_back(suffixes[i]);

      // Wonder if possible to give optimiser chance to somehow schedule these
      // together ie do both in separate arrays and then & them for the both?
      // Would that be faster than this where maybe we must wait for both?
      // Maybe change this to bitset after or something anyway rather than fat
      // heap vectors
      // const detail::TrieImpl* tp = nullptr;

      // TODO: refactor this?
      if (!tp)
      {
        break;
      }
      assert(tp);
      const detail::TrieImpl* p = nullptr;
      for (const auto pp: tp->children_)
      {
        if (pp->value_ == suffixes[i])
        {
          p = pp;
          break;
        }
      }
      if (!p)
      {
        continue;
      }
      // const auto p = detail::contains_and_further(*tp, std::string{suffixes[i]});

      const auto contains = detail::is_end_of_word(p);
      const auto further = detail::has_children(p);
      //fmt::print("For: {}, contains: {}, further: {}\n",
          // suffixes[i], contains, further);
      // this->contains_and_further
      // const auto contains = this->contains(stem);
      // const auto further = this->contains_prefix(stem);
      if (contains && further)
      {
        result.contains_and_further.push_back(i);
      } else if (contains)
      {
        result.contains.push_back(i);
      } else if (further)
      {
        result.further.push_back(i);
      }

      // stem.pop_back();
    }

    // return result;
  }

  template<class Container>
  void copy_insert_all(const Container& c)
  {
    for (const auto& val: c)
    {
      this->insert(val);
    }
  }
  [[nodiscard]] std::vector<std::string> as_vector() const
  {
    std::vector<std::string> traverse_words;
    this->traverse([&traverse_words] (const auto& word)
        {
          // //fmt::print("{}\n", word);
          traverse_words.push_back(word);
        });
    return traverse_words;
  }
  [[nodiscard]] std::size_t size() const
  {
    return size_;
  }

  Trie() noexcept
    : trie_()
    , size_()
    , cache_(&trie_)
  {
    fmt::print("Made trie with default cons at {}\n", this);
    fmt::print("trie_ {}, size_ {}, cache_ {}\n",
        &trie_, &size_, &cache_);
  }

  private:
  template<class T>
  static constexpr auto memes(long) -> decltype(std::false_type{})
  {
    return std::false_type{};
  }

  // I do not understand this jibberish. C++20 concepts can't come fast enough
  template<class T>
  static constexpr auto memes(int) -> decltype(
      std::begin(std::declval<T>()),
      std::end(std::declval<T>()),
      std::true_type{})
  {
    return std::true_type{};
  }

  public:
  // template<class T>
  template<class T, typename = std::enable_if_t<memes<T>(0)>>
  Trie(const T& c)
  : Trie(std::begin(c), std::end(c))
  {
  }

  void printme() const
  {
    std::cout << "Size of trie: " << this->size() << "\n";
    this->traverse(
        [] (const auto& v)
        {
        std::cout << v << "\n";
        });
  }

  template<class Iterator1, class Iterator2>
  Trie(Iterator1 first, const Iterator2 last)
    : trie_()
    , size_()
    , cache_(&trie_)
  {
    for (; first != last; std::advance(first, 1))
    {
      this->insert(*first);
    }
  }

  template<class F>
  void traverse(F&& f) const
  {
    detail::traverse(trie_, f);
  }

  // Trie(const Trie&) = delete;
  // Trie(Trie&&) = default;

  // Trie& operator=(const Trie&) = delete;
  // Trie& operator=(Trie&&) = default;

  // TODO: change this to move cons + swap
  Trie(const Trie&) = delete;
  // Trie(Trie&&) = default;
  Trie(Trie&& other) noexcept
    : trie_(std::move(other.trie_))
    , size_(std::move(other.size_))
    , cache_(std::move(other.cache_))
  {
    other.trie_.children_.clear();
    other.size_ = 0;
    // Hmm. This sucks.
    cache_.tp_ = &trie_;
  }

  Trie& operator=(const Trie&) = delete;
  // Trie& operator=(Trie&&) = default;
  Trie& operator=(Trie&& other) noexcept
  {
    trie_ = std::move(other.trie_);
    size_ = std::move(other.size_);
    cache_ = std::move(other.cache_);
    cache_.tp_ = &trie_;

    other.trie_.children_.clear();
    other.size_ = 0;
    return *this;
  }

  ~Trie()
  {
    std::cout << "DELETION OCURRING " << __PRETTY_FUNCTION__ << "\n";
  }

  private:
  const detail::TrieImpl* lookup(const std::string& word) const
  {
    auto [tp, i] = cache_.lookup(&trie_, word);
    for (; tp != nullptr; ++i)
    {
      assert(i <= word.size());
      if (i == word.size())
      {
        return tp;
      }
      tp = tp->lookup(word[i]);
      if (tp != nullptr)
      {
        cache_.append(tp);
      }
    }
    return nullptr;
  }

  detail::TrieImpl trie_;
  std::size_t size_;
  detail::Cache cache_;
};

// static_assert(std::is_trivially_default_constructible_v<Trie>);
static_assert(std::is_nothrow_default_constructible_v<Trie>);
static_assert(std::is_nothrow_move_constructible_v<Trie>);
static_assert(std::is_nothrow_move_assignable_v<Trie>);

} // namespace trie

struct TrieWrapper: public trie::Trie
{
  using trie::Trie::Trie;
  void contains_and_further(
      const std::string& stem,
      const std::string& suffixes,
      trie::ResultType& result_out) const
  {
    trie::Trie::contains_and_further(stem, suffixes, result_out);
    // return wordsearch_solver::Result{
      // std::move(result.contains),
      // std::move(result.further),
      // std::move(result.contains_and_further)};
  }
};

