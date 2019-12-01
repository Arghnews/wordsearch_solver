#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fmt/format.h>

// #include "jr_assert.h"

// using ResultType = wordsearch_solver::Result;

using ResultType = struct Result
{
  std::vector<std::size_t> contains;
  std::vector<std::size_t> further;
  std::vector<std::size_t> contains_and_further;
};

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

// inline std::pair<bool, bool>
[[nodiscard]] inline const TrieImpl*
contains_and_further(const TrieImpl& t, const std::string& word
    // std::string& cached_str,
    // std::vector<const TrieImpl*>& cached_ptrs
    )
{
  static std::string cached_str;
  static std::vector<const TrieImpl*> cached_ptrs;

  // return false;
  // auto it = word.begin();
  const auto* tp = &t;

  std::size_t i = 0;
  {
    const auto& cache = cached_str;
    assert(cache.size() == cache.size());
    // for (const auto c: cached_str)
    const auto shorter = std::min(cache.size(), word.size());
    while (i < shorter &&
        cache.at(i) == word.at(i))
    // for (; i < word.size(); ++i)
    {
      tp = cached_ptrs.at(i);
      // ++it;
      ++i;
    }
    assert(cache.size() == cached_ptrs.size());
    if (i != 0)
    {
      assert(tp);
      fmt::print("Cached output: {}, {}\n", i, tp->value_);
    }
  }

  cached_str.resize(i);
  cached_ptrs.resize(i);

  // In this loop, the tp pointers "lags" behind the iterator by one. This is so
  // when the iterator over "word" reaches the end, tp is at the previous and
  // valid pointer in the trie.
  // for (; tp != nullptr; ++it)
  for (; tp != nullptr; ++i)
  {
    assert(i <= word.size());
    if (i == word.size())
    // if (it == word.end())
    {
      return tp;
    }
    tp = tp->lookup(word.at(i));
    if (tp != nullptr)
    {
      // assert(tp && word.at(i) == tp->value_);
      // assert(tp && i + 1 == cached_str.size());
      cached_str.push_back(tp->value_);
      cached_ptrs.push_back(tp);
      // cached_str.push_back(
      // cached_ptrs.push_back(tp);
    }

    // if (tp == nullptr)
    // {
      // return nullptr;
    // } else if (it == word.end())
    // {
      // Have reached end of word
      // assert(tp);
      // return tp;
      // if (tp != nullptr)
      // {
        // return tp;
        // return {tp->is_end_of_word_, !tp->children_.empty()};
      // }
      // return {false, false};
      // return nullptr;
    // }
    // else if (tp == nullptr)
    // {
      // Ran out of letters, couldn't find end of word
      // return false;
      // return {false, false};
      // return nullptr;
    // }
  }
  return tp;
  assert(false && "Unreachable");
  //JR_ASSERT(false, "Unreachable");
}

inline bool is_end_of_word(const TrieImpl* tp)
{
  return tp && tp->is_end_of_word_;
}

inline bool has_children(const TrieImpl* tp)
{
  return tp && !tp->children_.empty();
}

inline std::pair<TrieImpl*, bool> insert(TrieImpl& t, const std::string& word)
{
  auto* tp = &t;
  for (const char c: word)
  {
    // fmt::print("Inserting {}\n", c);
    tp = tp->insert(c);
  }
  if (!tp->is_end_of_word_)
  {
    tp->is_end_of_word_ = true;
    return {tp, true};
  }
  return {tp, false};
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
    // fmt::print("\n--\n");
    const auto& [t, t_child_it] = v.back();
    if (t_child_it == t->children_.begin())
    {
      if (t->value_ != '\0') // special case for root, to remove
      {
        // fmt::print("Adding value: {}\n", t->value_);
        stack.push_back(t->value_);
      }
      if (t->is_end_of_word_)
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

template<class T>
const void* v(const T* t)
{
  return static_cast<const void*>(t);
}

template <class T1, class T2>
static constexpr std::size_t get_padding_between(const std::size_t n)
{
  const auto end_t1 = sizeof(T1) * n;
  auto padding = 0ULL;
  for (; (end_t1 + padding) % alignof(T2) != 0; ++padding) {}
  return padding;

  // alignas(std::max(alignof(T1), alignof(T2))) char arr[] = {'\0'};
  // void* end_of_t1 = arr + sizeof(T1) * n;
  // std::size_t space = std::numeric_limits<std::size_t>::max();
  // auto ret = std::align(alignof(T2), n, end_of_t1, space);
  // assert(ret != nullptr);
  // const auto padding_between_t1_t2 =
    // std::numeric_limits<std::size_t>::max() - space;
  // return padding_between_t1_t2;
}

template<class KeyType, class ValueType>
struct Cache
{

  // using key_type = Key;
  // using mapped_type = Value;
  using Key = KeyType;
  using Value = ValueType;
  // using Key = std::string;
  // using Value = const detail::TrieImpl*;

  Cache(const std::size_t n)
  : size_(0)
  , capacity_(n)
  , index_(0)
  , hits_(0)
  , misses_(0)
  {
    const auto padding = get_padding_between<Key, Value>(n);
    fmt::print("Setting padding to: {}\n", padding);
    const auto size = n * (sizeof(Key) + sizeof(Value)) + padding;
    fmt::print("Full size of mem: {}\n", size);
    // I'm unsure if this is necessary or if just alignof(Key) is enough
    const auto alignment = std::max(alignof(Key), alignof(Value));
    fmt::print("Alignof 1 vs 2: {} vs {}\n", alignof(Key), alignof(Value));
    fmt::print("Alignment max: {}\n", alignment);
    keys_ = std::aligned_alloc(alignment, size);
    fmt::print("Keys location: {}\n", v(keys_));
    // Don't expect this to fail unless OOM - could throw OOM error instead
    assert(keys_);
    values_ = static_cast<char*>(keys_) + sizeof(Key) * n + padding;
    fmt::print("Values location: {}\n", v(values_));
    fmt::print("Sizeof this + store: {} + {}\n", sizeof(*this),
        size);
  }

  template<class K>
  const Value* lookup(const K& key) const
  {
    fmt::print("Looking up key {}\n", key);
    for (auto p = static_cast<const Key*>(keys_);
        p != static_cast<const Key*>(keys_) + size_;
        ++p)
    {
      fmt::print("Comparing key to location {}\n", v(p));
      if (key == *p)
      {
        const auto i = std::distance(static_cast<const Key*>(keys_), p);
        const auto val_ptr = static_cast<const Value*>(values_) + i;
        fmt::print("Found at i: {}, val_ptr: {}\n", i, v(val_ptr));
        ++hits_;
        return val_ptr;
      }
    }
    fmt::print("Returning lookup nullptr\n");
    ++misses_;
    return nullptr;
  }

  template<class K, class... ValueArgs>
  void emplace(K&& keyarg, ValueArgs&&... valueargs)
  {
    fmt::print("Emplace called\n");
    // fmt::print("Inserting key: {}, value: {}\n", key, value);
    if (size_ == capacity_)
    {
      fmt::print("size == capacity {}, destroying elems at index_ first {}\n",
          capacity_, index_);
      // Destroy old element at this position
      (static_cast<const Key*>(keys_) + index_)->~Key();
      (static_cast<const Value*>(values_) + index_)->~Value();
    }
    fmt::print("Placement new creating values at key: {} and value: {}\n",
        v(static_cast<const Key*>(keys_) + index_),
        v(static_cast<const Value*>(values_) + index_));

    new (static_cast<Key*>(keys_) + index_) Key(std::forward<K>(keyarg));
    new (static_cast<Value*>(values_) + index_)
        Value(std::forward<ValueArgs>(valueargs)...);

    fmt::print("Index: {}, capacity: {}\n", index_, capacity_);
    ++index_ %= capacity_;
    fmt::print("After++ Index: {}, capacity: {}\n", index_, capacity_);
    if (size_ < capacity_)
    {
      ++size_;
    }
  }

  void insert(Key key, Value value)
  {
    fmt::print("Inserting key: {}, value: {}\n", key, value);
    if (size_ == capacity_)
    {
      fmt::print("size == capacity {}, destroying elems at index_ first {}\n",
          capacity_, index_);
      // Destroy old element at this position
      (static_cast<const Key*>(keys_) + index_)->~Key();
      (static_cast<const Value*>(values_) + index_)->~Value();
    }
    fmt::print("Placement new creating values at key: {} and value: {}\n",
        v(static_cast<const Key*>(keys_) + index_),
        v(static_cast<const Value*>(values_) + index_));

    new (static_cast<Key*>(keys_) + index_) Key(std::move(key));
    new (static_cast<Value*>(values_) + index_) Value(std::move(value));

    fmt::print("Index: {}, capacity: {}\n", index_, capacity_);
    ++index_ %= capacity_;
    fmt::print("After++ Index: {}, capacity: {}\n", index_, capacity_);
    if (size_ < capacity_)
    {
      ++size_;
    }
  }

  ~Cache()
  {
    fmt::print("Overall hits/misses: {}/{} -> {}\n", hits_, misses_,
        static_cast<double>(hits_) / static_cast<double>(misses_));
    for (auto i = 0ULL; i != size_; ++i )
    {
      (static_cast<Key*>(keys_) + i)->~Key();
      (static_cast<Value*>(values_) + i)->~Value();
    }
    std::free(keys_);
  }

  std::size_t size_;
  std::size_t capacity_;
  void* keys_;
  void* values_;
  std::size_t index_;
  mutable std::size_t hits_;
  mutable std::size_t misses_;
};

class Trie
{
  public:
  bool contains(const std::string& key) const
  {
    return detail::is_end_of_word(detail::contains_and_further(trie_, key));
    // // std::cout << __PRETTY_FUNCTION__ << " key: " << key << "\n";
    // const auto b = detail::contains(trie_, key);
    // // std::cout << "Returning " << b << "\n";
    // return b;
  }

  bool contains_prefix(const std::string& key) const
  {
    return detail::has_children(detail::contains_and_further(trie_, key));
    // // std::cout << __PRETTY_FUNCTION__ << " key: " << key << "\n";
    // const auto b = detail::contains_prefix(trie_, key);
    // // std::cout << "Returning " << b << "\n";
    // return b;
  }

  ResultType contains_and_further(const std::string stem,
      const std::string& suffixes) const
  {
    ResultType result;

    fmt::print("contains_and_further called with stem: {} and suffixes: {}\n",
        stem, suffixes);

    const detail::TrieImpl* tp = nullptr;
    // static Cache<std::string, const detail::TrieImpl*> cache{64};
    // const auto cache_entry = cache.lookup(stem);
    // if (cache_entry)
    // {
      // tp = *cache_entry;
    // } else
    // {
      tp = detail::contains_and_further(trie_, stem);
      // cache.insert(stem, tp);
    // }
    // const detail::TrieImpl* tp = nullptr;
    // static std::unordered_map<std::string, const detail::TrieImpl*> cache_;
    // auto cached = cache_.find(stem);
    // if (cached != cache_.end())
    // {
      // tp = cached->second;
      // fmt::print("Cache found {} -> {}\n", stem, v(tp));
      // assert(detail::contains_and_further(trie_, stem) == tp);
    // } else
    // {
      // tp = detail::contains_and_further(trie_, stem);
      // cache_.insert({stem, tp});
      // fmt::print("Cache not found, inserting {} -> {}\n", stem, v(tp));
    // }

    for (auto i = 0ULL; i < suffixes.size(); ++i)
    {
      // stem.push_back(suffixes[i]);

      // Wonder if possible to give optimiser chance to somehow schedule these
      // together ie do both in separate arrays and then & them for the both?
      // Would that be faster than this where maybe we must wait for both?
      // Maybe change this to bitset after or something anyway rather than fat
      // heap vectors
      // const detail::TrieImpl* tp = nullptr;
      const auto p = detail::contains_and_further(*tp, std::string{suffixes[i]});

      const auto contains = detail::is_end_of_word(p);
      const auto further = detail::has_children(p);
      fmt::print("For: {}, contains: {}, further: {}\n",
          suffixes[i], contains, further);
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

    return result;
  }

  bool insert(const std::string& word)
  {
    // If make pretty iterators can return iterator to inserted here
    const auto [tp, inserted] = detail::insert(trie_, word);
    auto old_size = size_;
    size_ += inserted;
    assert(size_ == old_size || size_ == old_size + 1);
    // std::cout << "Inserting " << word << " size now: " << size_ << "\n";
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
          // fmt::print("{}\n", word);
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
    // auto val = detail::insert(trie_, keys[0]);
    // std::cout << val.first << " " << val.second << "\n";
    // size_ += static_cast<bool>(val.second);
    // this->copy_insert_all(keys);
    this->printme();
  }

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

  Trie(const Trie&) = delete;
  Trie(Trie&&) = default;
  // Trie(Trie&& other)
    // : trie_(std::move(other.trie_))
    // , size_(other.size_)
  // {
    // other.trie_.children_.clear();
    // other.size_ = 0;
  // }

  Trie& operator=(const Trie&) = delete;
  Trie& operator=(Trie&&) = default;

  ~Trie()
  {
    std::cout << "DELETION OCURRING " << __PRETTY_FUNCTION__ << "\n";
  }
    // std::vector<detail::TrieImpl*> p;
    // p.reserve(this->size());
    // deleter(trie_, p);
    // assert(p.size() == this->size());
    // for (const auto child: p)
    // {
      // delete child;
    // }
  // }

  // static void deleter(const detail::TrieImpl& t, std::vector<detail::TrieImpl*>& p)
  // {
    // for (const auto child: t.children_)
    // {
      // p.push_back(child);
      // deleter(*child, p);
    // }
  // }

  private:
  detail::TrieImpl trie_;
  std::size_t size_;
};

// static_assert(std::is_trivially_default_constructible_v<Trie>);
static_assert(std::is_nothrow_default_constructible_v<Trie>);
static_assert(std::is_nothrow_move_constructible_v<Trie>);

