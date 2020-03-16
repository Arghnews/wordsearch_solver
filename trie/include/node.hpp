#ifndef TRIE_NODE_HPP
#define TRIE_NODE_HPP

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <bitset>
#include <cstddef>
#include <memory>
#include <ostream>
#include <vector>

namespace trie
{

class Node
{
  private:
  // I dislike this. We store a char here but really we're storing a [0, 26)
  // index. But why store an 8 byte size_t or 4 byte int when a 1 byte char will
  // do (although due to alignment we save nothing really). Intent? Really need
  // strong typedefs or equivalent. So much boilerplate to make own simple Index
  // type though.
  struct Edge
  {
    std::unique_ptr<Node> child;
    char c;
  };

  public:

  // TODO: try changing this to boost/llvm small_vector and see what difference
  using Edges = std::vector<Edge>;
  using EdgesIterator = Edges::const_iterator;

  Node() = default;
  Node(bool is_end_of_word);

  Node* add_char(char c);
  // void set_preceding(std::size_t preceding);
  void set_is_end_of_word(bool is_end_of_word);

  const Node* test(const char c) const;
  bool any() const;
  bool is_end_of_word() const;
  // PrecedingType preceding() const;
  friend std::ostream& operator<<(std::ostream& os, const Node& node);

  // This is here so the owning trie can print nodes and get their children.
  // Not delighted about it.
  const Edges& edges() const;

  private:

  Edges edges_;
  bool is_end_of_word_;
};

}

#endif // TRIE_NODE_HPP
