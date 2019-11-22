#include <cassert>
#include <iterator>
#include <iostream>
#include <initializer_list>
#include <string>
#include <vector>
#include <utility>

// #include "jr_assert.h"
#include "wordsearch_solver_defs.h"

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
  TrieImpl(TrieImpl&& other)
  {
    std::cout << "Move cons trie " << __PRETTY_FUNCTION__ << "\n";
    *this = std::move(other);
  }

  TrieImpl& operator=(const TrieImpl&) = delete;
  TrieImpl& operator=(TrieImpl&& other)
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

// Whether trie has words that start_with prefix, (now) including the prefix
// word itself
inline bool contains_prefix(const TrieImpl& t, const std::string& prefix)
{
  //JR_ASSERT(!prefix.empty(), "Empty word lookup?");

  // return false;
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
      // If at a valid node and either this is a word itself or it has children
      if (tp != nullptr && (tp->is_end_of_word_ || !tp->children_.empty()))
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
  //JR_ASSERT(false, "Unreachable");
  assert(false && "Unreachable");
}

inline std::pair<bool, bool>
contains_and_further(const TrieImpl& t, const std::string& word)
{
  // return false;
  auto it = word.begin();
  const auto* tp = &t;

  // In this loop, the tp pointers "lags" behind the iterator by one. This is so
  // when the iterator over "word" reaches the end, tp is at the previous and
  // valid pointer in the trie.
  for (;; ++it)
  {
    if (it == word.end())
    {
      // Have reached end of word
      if (tp != nullptr)
      {
        return {tp->is_end_of_word_, !tp->children_.empty()};
      }
      return {false, false};
    } else if (tp == nullptr)
    {
      // Ran out of letters, couldn't find end of word
      // return false;
      return {false, false};
    }
    tp = tp->lookup(*it);
  }
  assert(false && "Unreachable");
  //JR_ASSERT(false, "Unreachable");
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

class Trie
{
  // const detail::TrieImpl* contains(const std::string& key) const
  // {
    // // std::cout << __PRETTY_FUNCTION__ << " key: " << key << "\n";
    // const auto b = detail::contains(trie_, key);
    // // std::cout << "Returning " << b << "\n";
    // return b;
  // }
  // bool contains_prefix(const std::string& key) const
  // {
    // // std::cout << __PRETTY_FUNCTION__ << " key: " << key << "\n";
    // const auto b = detail::contains_prefix(trie_, key);
    // // std::cout << "Returning " << b << "\n";
    // return b;
  // }
  public:
  wordsearch_solver::Result contains_and_further(std::string stem,
      const std::string& suffixes) const
  {
    wordsearch_solver::Result result;

    for (auto i = 0ULL; i < suffixes.size(); ++i)
    {
      stem.push_back(suffixes[i]);

      // Wonder if possible to give optimiser chance to somehow schedule these
      // together ie do both in separate arrays and then & them for the both?
      // Would that be faster than this where maybe we must wait for both?
      // Maybe change this to bitset after or something anyway rather than fat
      // heap vectors
      const auto [contains, further] = detail::contains_and_further(trie_,
          stem);
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

      stem.pop_back();
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

  void printme() const
  {
    // std::cout << "Size of trie: " << this->size() << "\n";
    // this->traverse(
        // [] (const auto& v)
        // {
        // std::cout << v << "\n";
        // });
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
