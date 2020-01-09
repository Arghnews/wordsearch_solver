#ifndef TRIE_H
#define TRIE_H

#include <array>
#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <ostream>
#include <optional>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/for_each.hpp>
#include <range/v3/view/group_by.hpp>
#include <range/v3/view/indirect.hpp>
#include <range/v3/view/ref.hpp>
#include <range/v3/view/slice.hpp>
#include <range/v3/view/span.hpp>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/view.hpp>
#include <range/v3/view/zip.hpp>
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

#include <range/v3/all.hpp>

// #include "jr_assert.h"

// TODO: change const string& -> string_view or unify enable_if to only build
// from string and not views
// This might ACTUALLY be a case for inheritance what with the CompactTrie being
// a Trie?

namespace trie
{

namespace detail
{

struct TrieImpl
{
  std::vector<TrieImpl*> children_;
  char value_;
  bool is_end_of_word_;

  auto as_compressed() const
  {
    std::bitset<27> b;
    for (const auto child: children_)
    {
      // From ascii to 0
      assert(97 <= child->value_ && child->value_ < 123 &&
          "Must be lowercase ascii");
      b.set(static_cast<std::size_t>(child->value_ - 97));
    }
    b.set(26, is_end_of_word_);
    return b;
  }

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
    os << trie.value_;
    if (trie.is_end_of_word_) os << " end_of_word";
    os << " children: (" << trie.children_.size() << "): ";
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
  const TrieImpl* tp_;
  mutable std::string cached_str_;
  mutable std::vector<const TrieImpl*> cached_ptrs_;
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
  lookup(const TrieImpl* tp, const std::string_view word) const
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
inline void traverse_impl(const TrieImpl& root, F&& f)
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
      f(stack, t->is_end_of_word_);
      // if (t->is_end_of_word_)
      // {
        // Coroutine anyone?
        // //fmt::print("Output: {}\n", stack);
      // }
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

template<class F>
inline void traverse(const TrieImpl& root, F&& f)
{
  traverse_impl(root,
      [ff = std::forward<F>(f)] (const auto& str, const bool is_end_of_word)
      {
        if (is_end_of_word) ff(str);
      });
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

  bool insert(const std::string_view word)
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

  template<class OutputIndexIterator>
  void contains_and_further(const std::string& stem,
      const std::string& suffixes,
      OutputIndexIterator contains_out_it,
      OutputIndexIterator further_out_it,
      OutputIndexIterator contains_and_further_out_it) const
  {
    // fmt::print("contains_and_further called with stem: {} and suffixes: {}\n",
        // stem, suffixes);

    const auto* const tp = this->lookup(stem);
    if (!tp) return;
    for (auto i = 0ULL; i < suffixes.size(); ++i)
    {
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

      const auto contains_word = detail::is_end_of_word(p);
      const auto further_than_word = detail::has_children(p);
      if (contains_word && further_than_word)
      {
        *contains_and_further_out_it++ = i;
      } else if (contains_word)
      {
        *contains_out_it++ = i;
      } else if (further_than_word)
      {
        *further_out_it++ = i;
      }
    }
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

  template<class T>
  explicit Trie(const T& c)
  : Trie(std::begin(c), std::end(c))
  {
  }

  template<class T,
    typename = std::enable_if_t<std::is_convertible_v<
      T, const std::string_view>>>
  Trie(std::initializer_list<T> words)
  : Trie(words.begin(), words.end())
  {
  }

  template<class Iterator1, class Iterator2>
  Trie(Iterator1 first, const Iterator2 last)
    : trie_()
    , size_()
    , cache_(&trie_)
  {
    // decltype(first)::name;
    // const std::string& ref = *first;
    for (; first != last; std::advance(first, 1))
    {
      this->insert(*first);
    }
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

  friend std::ostream& operator<<(std::ostream& os, const Trie& c)
  {
    os << "Size of trie: " << c.size() << "\n";
    std::vector<const detail::TrieImpl*> q{&c.trie_};
    std::vector<const detail::TrieImpl*> level;
    auto i = 0;
    while (!q.empty())
    {
      auto val = ranges::views::all(q) | ranges::views::indirect;
      fmt::print("Level {}: {}\n", i++, val);
      for (const auto& trieimpl: q)
      {
        level.insert(level.end(), trieimpl->children_.begin(),
            trieimpl->children_.end());
      }
      q = std::move(level);
    }
    return os;
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

  friend class CompactTrieImpl;
};

static auto make_rows(
    const std::vector<std::bitset<27>>& compressed,
    const std::vector<std::size_t>& levels)
{
  // Difficult (nigh on impossible) to use actual types and not auto/templates
  // here as the type of this vector is "ranges::subrange<...,
  // (ranges::subrange_kind)1>" and that last bit I'm unsure what it is.
  // Concepts can't come soon enough.
  // TODO: make actual compressed_ member this
  return ranges::views::zip(levels, ranges::views::tail(levels))
    | ranges::views::transform(
      [&] (const auto& level)
      {
        const auto [first, last] = level;
        return ranges::subrange{
          compressed.begin() + static_cast<long>(first),
          compressed.begin() + static_cast<long>(last)};
      })
    | ranges::to<std::vector>();
}

class CompactTrieImpl
{
  public:
  using Compressed = std::bitset<27>;
  using CompressedVectorIterator = std::vector<Compressed>::const_iterator;
  using Rows = std::invoke_result_t<
    decltype(make_rows),
    std::vector<Compressed>,
    std::vector<std::size_t>>;
  using RowIterator = Rows::const_iterator;

  CompactTrieImpl() = default;

  static std::bitset<26> letters_bitset(const std::bitset<27>& bits)
  {
    return std::bitset<26>{bits.to_ullong()};
  };

  static bool is_end_of_word(const std::bitset<27>& bits)
  {
    return bits.test(26);
  };

  // Returns a tuple, if the bool is false then word wasn't found, values
  // unspecified.
  // If word was found, returns {true, iterator_to_last_letter,
  // iterator_to_next_row_position}

  // This function is tricky to implement right. Or perhaps I'm such a n00b with
  // range-v3 (which I am). But it's turned out quite messily.
  // FIXME: rewrite this mess

  template<class... Args>
  static void log(Args&&... args)
  {
    if (0)
    {
      fmt::print(std::forward<Args>(args)...);
    }
  }

  // template<class Rows>
  static
  std::tuple<bool, CompressedVectorIterator, CompressedVectorIterator>
  find_word(const Rows& rows, const std::string& word)
  {
    log("\nFinding word: {}\n", word);
    // If the word is not found, returns an empty optional
    assert(!word.empty() && "Don't want to handle empty string search");
    // const auto last = compressed_.end();
    if (word.empty() /*|| word.size() > rows.size()*/)
    {
      log("Word was empty, didn't find it\n");
      return {false, {}, {}};
    }
    // rows.size() > word.size()

    if (!rows.empty())
    {
      // First row should be root
      assert(rows.begin()->size() == 1);
    }
    assert(rows.size() >= 1);
    assert(rows.begin()->size() >= 1);

    CompressedVectorIterator elem_it;
    CompressedVectorIterator next_elem_it = rows.begin()->begin();
    unsigned long letters_before = 0;
    auto row_it = rows.begin();
    const auto row_end = (--rows.end())->end();

    for (const auto c: word)
    {
      assert(97 <= c && c < 123 && "Must be lowercase ascii");
      log("\n");
      elem_it = next_elem_it;
      const auto letter = static_cast<std::size_t>(c) - 97;
      // assert(row.size() > static_cast<unsigned long>(letters_before));
      const auto letter_bits = letters_bitset(*elem_it);

      log("Testing letter {} ({})\n", c, int(c));

      if (!letter_bits.test(letter))
      {
        log("Didn't find it\n");
        return {false, {}, {}};
      }

      // Count nodes before and including this
      log("letter_bits {}\n", letter_bits);
      // log("(letter_bits >> {}) {}\n", letter, letter_bits >> letter);
      // log("(letter_bits >> {} + 1) {}\n", letter,
      // letter_bits >> (letter + 1));
      log("(letter_bits << {}) {}\n", letter, letter_bits << (26 - letter));
      const auto this_node = (letter_bits << (26 - letter)).count();
      log("letter_bits >> (letter + 1)).count(): {}\n", this_node);

      // Count nodes before this
      // log("row_it->begin() to elem_it\n");
      const auto prior_nodes = std::accumulate(
          row_it->begin(), elem_it,
          0UL, [] (const auto acc, const auto& node)
          {
            log("Adding acc {} to count {}\n", acc, letters_bitset(node));
            return acc + letters_bitset(node).count();
          });
      letters_before = this_node + prior_nodes;
      log("letters_before {} = this_node {} + prior_nodes {}\n",
          letters_before, this_node, prior_nodes);

      ++row_it;
      if (row_it == rows.end())
      {
        next_elem_it = row_end;
      }
      else
      {
        next_elem_it = std::next(row_it->begin(),
            static_cast<long>(letters_before));
      }
    }

    log("Found it!\n");
    return {true, elem_it, next_elem_it};
  }

  explicit CompactTrieImpl(const Trie& t)
  {
    std::vector<const detail::TrieImpl*> q{&t.trie_};
    std::vector<const detail::TrieImpl*> next_level{};
    std::vector<std::size_t> levels;

    compressed_.reserve(t.size() * sizeof(Compressed));
    std::size_t size = 0;

    while (!q.empty())
    {
      fmt::print("Level {}\n", levels.size());
      const auto level_size = q.size();
      levels.push_back(compressed_.size());
      for (const auto& item: q)
      {
        fmt::print("Pushing back {}\n", print_bitset(item->as_compressed()));
        compressed_.push_back(item->as_compressed());
        next_level.insert(next_level.end(), item->children_.begin(),
            item->children_.end());
      }
      size += level_size;

      q = std::move(next_level);
    }

    rows_ = make_rows(compressed_, levels);

    fmt::print("{}\n", *this);
    size_ = size;

    // ranges::views::zip(levels, ranges::views::tail(levels)) |
      // ranges::transform(
      // ranges::views::slice(ranges::views::all(compressed_),

    // Given a vector of subranges where each subrange is a row, and a word,
    // returns a std::optional<iterator>. If the word is found, the optional
    // contains the iterator pointing to the last character.
    // If the word is not found, returns an empty optional
    // auto search = [&letters_bitset] (const auto& rows, const std::string word)
      // -> std::optional<typename
      // std::decay_t<decltype(rows)>::value_type::iterator>
    // {
      // // return {};
      // // const std::string word = "hi there";
      // assert(!word.empty() && "Don't want to handle empty string search");
      // // const auto last = compressed_.end();
      // if (word.empty() || word.size() > rows.size()) return {};

      // using T = typename std::decay_t<decltype(rows)>::value_type::iterator;
      // T it{};
      // for (const auto& [c, row]: ranges::views::zip(word, rows))
      // {
        // const auto letter = static_cast<std::size_t>(c) - 97;
        // it = ranges::find_if(row,
            // [&letters_bitset, letter] (const auto& elem)
            // {
              // return letters_bitset(elem).test(letter);
            // });
        // if (it == row.end())
        // {
          // return {};
        // }
      // }
      // return std::optional<T>{it};
    // };

    // // std::optional<decltype(rows)::value_type::iterator> d{};
    // auto o = search(rows, "hi there");

    // std::vector<int> a;
    // auto lst0 = ranges::views::ints | ranges::views::transform([](int i) {
        // return i * i; }) |
            // ranges::views::take(10) | ranges::to<std::vector>();

    // ranges::for_each(ranges::views::all(a), [] (const auto e)
        // { return e; }) | ranges::to<std::vector<int>>();
    // ranges::to<std::vector>(res);

    // ranges::for_each(ranges::views::all(rows),
        // [](const auto& elem) { return *elem; });
    // rows | ranges::for_each([](const auto& elem) { return *elem; });

    // for (const auto [first_index, last_index]: ranges::views::zip(levels_,
          // ranges::views::tail(levels_)))
    // {
      // using CompressedVectorIterator = std::vector<Compressed>::const_iterator;
      // std::vector<CompressedVectorIterator> row{
        // compressed_.begin() + first_index, compressed_.begin() + last_index};
      // std::vector<CompressedVectorIterator> children{
        // compressed_.begin() + first_index, compressed_.begin() + last_index};
      // for (const auto it: row)
      // {
      // }

      // ranges::views::slice(ranges::views::all(compressed_),
            // static_cast<long>(first_index),
            // static_cast<long>(last_index));
      // fmt::print("{}\n", view);
    // }

    // ranges::views::enumerate(compressed_) | ranges::views::group_by(
        // [&] (const auto& index_elem0, const auto& index_elem1)
        // {
          // const auto& [index0, elem0] = index_elem0;
          // const auto& [index1, elem1] = index_elem1;

        // });
    // assert(t.size() == size);
  }

  auto size() const
  {
    return size_;
  }

  auto end() { return compressed_.end(); }
  // auto end() { return compressed_.end(); }

  // std::vector<Compressed>::const_iterator
    // search(const std::string& needle) const
  // {

    // std::vector<int> a;
    // // ranges::views::slice(
    // ranges::subrange raange{a.begin(), a.end()};
    // search_for_char('a', raange);

    // // search_for_char('a', );
    // // ranges::slice_view(
        // // compressed_,
        // // compressed_.begin(), compressed_.begin() + 2);
    // // decltype(ranges::views::all(compressed_))::n;
    // // decltype(ranges::views::all(compressed_.begin(), compressed_.begin() + 2))::n;

    // auto log = [&] (auto&&... args)
    // {
      // if (1) fmt::print(std::forward<decltype(args)>(args)...);
    // };
    // auto first = needle.begin();
    // const auto last = needle.end();
    // if (first == last) return compressed_.end();

    // assert(compressed_.begin() != compressed_.end());

    // auto count_set = [] (auto first, const auto last)
    // {
      // assert(first <= last);
      // auto total = 0L;
      // for (; first != last; ++first)
      // {
        // // Count all except end_of_word bit
        // total += (*first << 1).count();
      // }
      // return total;
    // };

    // log("\nSearching for {}", needle);

    // // auto it = compressed_.begin();
    // auto row_start = compressed_.begin();
    // auto row_end = std::next(row_start);
    // auto row_offset = 0L;
    // auto current_pos = row_start;

    // assert(first != last);

    // for (; first != last; ++first)
    // {
      // log("\nIn iter for *first: {} at index: {}\n", *first,
          // std::distance(compressed_.begin(), row_start));

      // log("row_start {}\n", std::distance(compressed_.begin(), row_start));
      // log("row_end {}\n", std::distance(compressed_.begin(), row_end));
      // log("row_offset {}\n", row_offset);
      // log("current_pos {}\n",
         // std::distance(compressed_.begin(), current_pos));

      // const auto total_before = count_set(row_start, row_start + row_offset);
      // log("total_before {}\n", total_before);

      // const auto letter_index = static_cast<std::size_t>(*first - 97);
      // log("letter_index {}\n", letter_index);
      // const std::bitset<26> b = (*(row_start + row_offset)).to_ullong();
      // log("Bitset of letter without end_of_word {}\n", b);

      // if (first + 1 == last && b.test(letter_index))
      // {
        // log("Done! Found!\n");
        // // return current_pos;
      // }
      // if (!b.test(letter_index))
      // {
        // // log("No letter_index {} in b {} ({}), returning end\n",
            // // letter_index, b, print_bitset(b));
        // // return compressed_.end();
      // }

      // // Nasty casts just for now
      // log("b: {}\n", b);
      // log("before_bitset: {}\n", b >> (letter_index));
      // log("after_bitset: {}\n", b << (b.size() - letter_index));
      // const auto before = (unsigned long)((b >> (letter_index)).count());
      // const auto after = (unsigned long)((b << (b.size() - letter_index)).count());
      // log("before: {}, after: {}\n", before, after);

      // const auto total_after = count_set(row_start + row_offset + 1, row_end);
      // log("total_before: {}, total_after: {}\n", total_before, total_after);

      // row_start = row_end;
      // row_offset = total_before + before - 1;
      // row_end = row_start + row_offset + 1 + total_after + after;
      // current_pos = row_start + row_offset;
    // }

    // fmt::print("[Was unreachable]\n");
    // // assert(false && "Unreachable");
    // return current_pos;
  // }

  [[nodiscard]] bool contains(const std::string& key) const
  {
    assert(!key.empty() && "Whether or not searching for the empty string "
        "makes sense, for now it's unsupported as it's likely a bug");
    const auto [found, _, next_it] = find_word(rows_, key);
    if (found)
    {
      return is_end_of_word(*next_it);
    }
    return false;
    // const auto it = this->search(key);
    // return it != compressed_.end() ? it->test(26) : false;
  }

  [[nodiscard]] bool further(const std::string& key) const
  {
    assert(!key.empty() && "Whether or not searching for the empty string "
        "makes sense, for now it's unsupported as it's likely a bug");
    const auto [found, it, next_it] = find_word(rows_, key);
    if (found)
    {
      return letters_bitset(*next_it).count() > 0;
    }
    return false;
    // const auto it = this->search(key);
    // if (it == compressed_.end()) return false;
    // return std::bitset<26>{it->to_ullong()}.count();
  }

  template<class OutputIndexIterator>
  void contains_and_further(const std::string& stem,
      const std::string& suffixes,
      OutputIndexIterator contains_out_it,
      OutputIndexIterator further_out_it,
      OutputIndexIterator contains_and_further_out_it) const
  {
    auto word = stem + '\0';
    for (auto i = 0ULL; i < suffixes.size(); ++i)
    {
      word.back() = suffixes[i];
      const bool contains = this->contains(word);
      const bool further = this->further(word);
      if (contains && further)
      {
        *contains_and_further_out_it++ = i;
      } else if (contains)
      {
        *contains_out_it++ = i;
      } else if (further)
      {
        *further_out_it++ = i;
      }
    }
    word.pop_back();
  }

  static std::string print_bitset(std::bitset<26> b)
  {
    std::string s;
    for (auto i = 0ULL; i < b.size(); ++i)
    {
      if (b.test(i)) s.push_back(char(i + 97));
    }
    return s;
  }

  static std::string print_bitset(std::bitset<27> b)
  {
    std::string s;
    s += "{";
    for (auto i = 0ULL; i < b.size() - 1; ++i)
    {
      if (b.test(i)) s.push_back(char(i + 97));
    }
    if (b.test(b.size() - 1))
    {
      s += "|";
    }
    // os << ' ';
    s += "} ";
    return s;
  }

  friend std::ostream& operator<<(std::ostream& os, const CompactTrieImpl& c)
  {
    ranges::for_each(c.rows_,
        [&] (const auto& row)
        {
          ranges::for_each(row,
              [&] (const auto& elem)
              {
                os << print_bitset(elem);
              });
          os << "\n";
        }
      );
    return os;
  }

  std::vector<Compressed> compressed_;
  std::size_t size_;
  // Yikes...
  std::invoke_result_t<
    decltype(make_rows),
    std::vector<Compressed>,
    std::vector<std::size_t>
      > rows_;
};

// Nasty wrapper for now to see
class CompactTrie
{
  public:
  CompactTrie() = default;

  template<class T>
  explicit CompactTrie(const T& c)
  : CompactTrie(std::begin(c), std::end(c))
  {}

  template<class T,
    typename = std::enable_if_t<std::is_convertible_v<
      T, const std::string_view>>>
  CompactTrie(std::initializer_list<T> words)
  : CompactTrie(words.begin(), words.end())
  {}

  // This is the constructor that does the work
  template<class Iterator1, class Iterator2>
  CompactTrie(Iterator1 first, const Iterator2 last)
  : trie_(first, last)
  , compact_trie_(trie_)
  , to_insert_()
  {}

  [[nodiscard]] std::vector<std::string> as_vector() const
  {
    return trie_.as_vector();
  }

  template<class F>
  void traverse(F&& f) const
  {
    trie_.traverse(std::forward<F>(f));
  }

  bool insert(const std::string_view word)
  {
    const auto inserted = trie_.insert(word);
    if (inserted)
    {
      to_insert_.push_back(std::string{word});
    }
    return inserted;
  }

  void rebuild_if_updated() const
  {
    if (!to_insert_.empty())
    {
      compact_trie_ = CompactTrieImpl{trie_};
      to_insert_.clear();
    }
  }

  [[nodiscard]] bool contains(const std::string& key) const
  {
    rebuild_if_updated();
    const auto ret = compact_trie_.contains(key);
    assert(ret == trie_.contains(key));
    return ret;
  }

  [[nodiscard]] bool further(const std::string& key) const
  {
    rebuild_if_updated();
    // fmt::print("Testing if further has key: \"{}\"\n", key);
    const auto ret = compact_trie_.further(key);
    // fmt::print("Testing if further {}, trie: {}, ctrie: {}\n", key,
        // trie_.further(key), ret);
    assert(ret == trie_.further(key));
    return ret;
  }

  template<class OutputIndexIterator>
  void contains_and_further(const std::string& stem,
      const std::string& suffixes,
      OutputIndexIterator contains_out_it,
      OutputIndexIterator further_out_it,
      OutputIndexIterator contains_and_further_out_it) const
  {
    rebuild_if_updated();
    compact_trie_.contains_and_further(stem, suffixes,
        contains_out_it, further_out_it, contains_and_further_out_it);
  }

  std::size_t size() const
  {
    return trie_.size();
  }

  friend std::ostream& operator<<(std::ostream& os, const CompactTrie& ct)
  {
    ct.rebuild_if_updated();
    os << "Trie: " << ct.trie_ << "\n";
    os << "CompactTrie: " << ct.compact_trie_ << "\n";
    return os;
  }

  Trie trie_;
  mutable CompactTrieImpl compact_trie_;
  mutable std::vector<std::string> to_insert_;
};

// template Trie::Trie(const std::initializer_list<const char*>);
// template Trie::Trie(const std::initializer_list<std::string>);
// template Trie::Trie(const std::initializer_list<std::string_view>);

// static_assert(std::is_trivially_default_constructible_v<Trie>);
static_assert(std::is_nothrow_default_constructible_v<Trie>);
static_assert(std::is_nothrow_move_constructible_v<Trie>);
static_assert(std::is_nothrow_move_assignable_v<Trie>);

} // namespace trie

#endif
