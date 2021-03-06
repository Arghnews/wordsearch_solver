#ifndef TRIE_NODE_HPP
#define TRIE_NODE_HPP

#include <boost/container/small_vector.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
// #include "llvm/ADT/SmallVector.h"

#include <bitset>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <vector>

namespace trie {

/** A vector of edges to child nodes, sorted by edge child node character value,
 * and a bool indicating whether or not a word terminates at this node.
 */
class Node {
private:
  // I dislike this. We store a char here but really we're storing a [0, 26)
  // index. But why store an 8 byte size_t or 4 byte int when a 1 byte char will
  // do (although due to alignment we save nothing really). Intent? Really need
  // strong typedefs or equivalent. So much boilerplate to make own simple Index
  // type though.
  struct Edge {
    std::unique_ptr<Node> child;
    char c;
  };

public:
  // TODO: try changing this to boost/llvm small_vector and see what difference
  // using Edges = std::vector<Edge>;
  using Edges = boost::container::small_vector<Edge, 4>;
  // using Edges = llvm::SmallVector<Edge, 4>;
  using EdgesIterator = Edges::const_iterator;

  Node() = default;
  Node(bool is_end_of_word);

  /** Inserts a child node corresponding to the character @p c if it doesn't
   * exist, and returns a pointer to the child node.
   */
  Node* add_char(char c);
  void set_is_end_of_word(bool is_end_of_word);

  const Node* test(const char c) const;
  bool any() const;
  bool is_end_of_word() const;
  // PrecedingType preceding() const;
  friend std::ostream& operator<<(std::ostream& os, const Node& node);

  // This is here so the owning trie can print nodes and get their children.
  // Not delighted about it.
  // Don't want to make the Trie a friend as introduces circular dependency.
  // Could use Passkey idiom I believe?
  // Since this is const, leave it for now.
  const Edges& edges() const;

private:
  Edges edges_;
  bool is_end_of_word_;
};

// https://stackoverflow.com/a/18405291/8594193
// Assuming the currently used boost::container::small_vector follows
// std::vector, this type satisfies the type trait,
// std::is_copy_constructible_v even though it's a container of
// std::unique_ptr s and therefore clearly should not be copy constructible,
// at least now with a defaulted copy constructor
// static_assert(std::is_copy_constructible_v<Edges>);
// static_assert(std::is_copy_constructible_v<Node>);

const Node* search(const Node& node, std::string_view word);

} // namespace trie

#endif // TRIE_NODE_HPP
