#include "compact_trie.hpp"

#include <prettyprint.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <range/v3/view/all.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/subrange.hpp>
#include <range/v3/view/zip.hpp>

#include <cstddef>
#include <iterator>
#include <initializer_list>
#include <string_view>
#include <utility>

// This might ACTUALLY be a case for inheritance what with the CompactTrie being
// a Trie?
// TODO: test this with const_iterator not a std::tuple, and try a simple user
// defined struct/pointer to make trivial type to help the optimiser? Not even
// sure if "trivial" means what I think it does anyway, remove this likely..
// TODO: maybe look into units library for the ascii/index conversion stuff, as
// that has already wasted a significant amount of time with offset stuff

namespace
{

template<class Range>
auto make_adjacent_view(Range&& rng)
{
  return ranges::views::zip(
      ranges::views::all(std::forward<Range>(rng)),
      ranges::views::all(std::forward<Range>(rng))
        | ranges::views::drop(1));
}

template<class DataView, class RowIndexes>
auto make_row_view(DataView&& data_view, RowIndexes&& row_indexes)
{
  return make_adjacent_view(row_indexes)
    | ranges::views::transform(
      [data_view = std::forward<DataView>(data_view)] (const auto row)
      {
        const auto [row_start, row_end] = row;
        return ranges::subrange(row_start, row_end);
        // return ranges::views::slice(data_view, static_cast<long>(row_start),
            // static_cast<long>(row_end));
      });
}

}

namespace compact_trie
{

CompactTrie::CompactTrie(const std::initializer_list<std::string_view>& words)
  : CompactTrie(ranges::views::all(words))
{}

CompactTrie::CompactTrie(const std::initializer_list<std::string>& words)
  : CompactTrie(ranges::views::all(words))
{}

CompactTrie::CompactTrie(const std::initializer_list<const char*>& words)
  : CompactTrie(ranges::views::all(words)
      | ranges::views::transform(
        [] (const auto string_literal)
        {
        return std::string_view{string_literal};
        }
        ))
{}

bool CompactTrie::contains(const std::string_view word,
    const ranges::subrange<NodesIterator> nodes,
    const ranges::subrange<RowsIterator> rows) const
{
  const auto [node_it, rows_it] = detail::search(word, nodes, rows);
  if (node_it == nodes.end())
  {
    return false;
  }
  const auto letters_consumed = static_cast<std::size_t>(
      std::distance(rows.begin(), rows_it));
  return letters_consumed == word.size() && node_it->is_end_of_word();
}

bool CompactTrie::contains(const std::string_view word) const
{
  return contains(word, ranges::subrange(nodes_), ranges::subrange(rows_));
}

bool CompactTrie::further(const std::string_view word,
    const ranges::subrange<NodesIterator> nodes,
    const ranges::subrange<RowsIterator> rows) const
{
  const auto [node_it, rows_it] = detail::search(word, nodes, rows);
  if (node_it == nodes.end())
  {
    return false;
  }
  const auto letters_consumed = static_cast<std::size_t>(
      std::distance(rows.begin(), rows_it));
  return letters_consumed == word.size() && node_it->any();
}

bool CompactTrie::further(const std::string_view word) const
{
  return further(word, ranges::subrange(nodes_), ranges::subrange(rows_));
}

std::size_t CompactTrie::size() const
{
  return size_;
}

std::ostream& operator<<(std::ostream& os, const CompactTrie& ct)
{
  fmt::memory_buffer buff{};
  fmt::format_to(buff, "Size: {}\n", ct.size());
  for (const auto [i, row]: ranges::views::enumerate(make_row_view(ct.nodes_,
          ct.rows_)))
  {
    fmt::format_to(buff, "Row: {}\n", i);
    for (const auto& node: row)
    {
      fmt::format_to(buff, "{}", node);
    }
    fmt::format_to(buff, "\n");
  }
  fmt::format_to(buff, "Rows: {}\n", ranges::views::all(ct.rows_)
      | ranges::views::transform(
        [&ct] (const auto row_it)
        {
          return std::distance(ct.nodes_.begin(), row_it);
        }));
  return os << fmt::to_string(buff);
}

namespace detail
{

CompactTrie::NodesIterator follow(const CompactTrie::NodesIterator node_it,
    CompactTrie::RowsIterator rows_it,
    const std::uint8_t index,
    const CompactTrie::NodesIterator nodes_end,
    const CompactTrie::RowsIterator rows_end)
{
  assert(node_it != nodes_end);
  assert(rows_it != rows_end);

  if (!node_it->test(index) || ++rows_it == rows_end)
  {
    return nodes_end;
  }

  const auto before_node = node_it->preceding();
  const auto before_in_node = node_it->bits_on_before(index);
  const auto preceding = before_node + before_in_node;

  return *rows_it + static_cast<long>(preceding);
}

CompactTrie::const_iterator search(const std::string_view word,
    const ranges::subrange<CompactTrie::NodesIterator> nodes,
    const ranges::subrange<CompactTrie::RowsIterator> rows)
{
  if (nodes.empty())
  {
    return {nodes.end(), rows.end()};
  }

  auto it = nodes.begin();
  auto rows_it = rows.begin();

  // const bool use_cache = nodes.begin() == nodes_.begin();
  // if (use_cache)
  // {
    // if (const auto cached_result = cache_.lookup(word))
    // {
      // const auto [elems_consumed, value] = *cached_result;
      // word.remove_prefix(elems_consumed);
      // std::tie(it, rows_it) = value;
    // }
  // }

  for (const auto c: word)
  {
    // fmt::print("\nFollowing: {}\n", c);

    const auto next_it = follow(it, rows_it, static_cast<std::uint8_t>(
          c), nodes.end(), rows.end());
    if (next_it == nodes.end())
    {
      assert(it != nodes.end());
      assert(rows_it != rows.end());
      return {it, rows_it};
    }
    // fmt::print("Node now: {}, row: {}, row_index: {}\n", *next_it,
        // rows_it - rows.begin(), it - nodes.begin() + *rows_it);
    it = next_it;
    ++rows_it;
    // if (use_cache)
    // {
      // cache_.append(c, {it, rows_it});
    // }
  }
  assert(it != nodes.end());
  assert(rows_it != rows.end());
  return {it, rows_it};
}

}


}

// namespace trie
// {

// struct Compressed
// {
  // std::bitset<27> bits_;
  // unsigned short int preceding_;
  // using PrecedingType = unsigned short int;
// };

// static std::bitset<26> letters_bitset(const Compressed& bits)
// {
  // return std::bitset<26>{bits.bits_.to_ullong()};
// }

// static bool is_end_of_word(const Compressed& bits)
// {
  // return bits.bits_.test(26);
// }

// static auto make_rows(
    // const std::vector<Compressed>& compressed,
    // const std::vector<std::size_t>& levels)
// {
  // // Difficult (nigh on impossible) to use actual types and not auto/templates
  // // here as the type of this vector is "ranges::subrange<...,
  // // (ranges::subrange_kind)1>" and that last bit I'm unsure what it is.
  // // Concepts can't come soon enough.
  // // TODO: make actual compressed_ member this
  // return ranges::views::zip(levels, ranges::views::tail(levels))
    // | ranges::views::transform(
      // [&] (const auto& level)
      // {
        // const auto [first, last] = level;
        // return ranges::subrange{
          // compressed.begin() + static_cast<long>(first),
          // compressed.begin() + static_cast<long>(last)};
      // })
    // | ranges::to<std::vector>();
// }

// class CompactTrieImpl
// {
  // public:
  // // using Compressed = std::bitset<27>;

  // using CompressedVectorIterator = std::vector<Compressed>::const_iterator;
  // using Rows = std::invoke_result_t<
    // decltype(make_rows),
    // std::vector<Compressed>,
    // std::vector<std::size_t>>;
  // using RowIterator = Rows::const_iterator;

  // CompactTrieImpl() = default;

  // // Returns a tuple, if the bool is false then word wasn't found, values
  // // unspecified.
  // // If word was found, returns {true, iterator_to_last_letter,
  // // iterator_to_next_row_position}

  // // This function is tricky to implement right. Or perhaps I'm such a n00b with
  // // range-v3 (which I am). But it's turned out quite messily.
  // // FIXME: rewrite this mess

  // template<class... Args>
  // static void log(Args&&... args)
  // {
    // if (0)
    // {
      // fmt::print(std::forward<Args>(args)...);
    // }
  // }

  // static
  // std::tuple<bool, CompressedVectorIterator, CompressedVectorIterator,
    // RowIterator>
  // find_word(const Rows& rows, const std::string& word)
  // {
    // //log("\nFinding word: {}\n", word);
    // // If the word is not found, returns an empty optional
    // assert(!word.empty() && "Don't want to handle empty string search");
    // // const auto last = compressed_.end();
    // if (word.empty() [>|| word.size() > rows.size()<])
    // {
      // //log("Word was empty, didn't find it\n");
      // // std::cerr << "Passed empty word! in find_word" << "\n";
      // return {false, {}, {}, {}};
    // }
    // // rows.size() > word.size()

    // if (!rows.empty())
    // {
      // // First row should be root
      // assert(rows.begin()->size() == 1);
    // }
    // assert(rows.size() >= 1);
    // assert(rows.begin()->size() >= 1);

    // CompressedVectorIterator elem_it;
    // CompressedVectorIterator next_elem_it = rows.begin()->begin();
    // unsigned long letters_before = 0;
    // auto row_it = rows.begin();
    // const auto row_end = (--rows.end())->end();

    // for (const auto c: word)
    // {
      // assert(97 <= c && c < 123 && "Must be lowercase ascii");
      // //log("\n");
      // elem_it = next_elem_it;
      // const auto letter = static_cast<std::size_t>(c) - 97;
      // // assert(row.size() > static_cast<unsigned long>(letters_before));
      // const auto letter_bits = letters_bitset(*elem_it);

      // //log("Testing letter {} ({})\n", c, int(c));

      // if (!letter_bits.test(letter))
      // {
        // //log("Didn't find it\n");
        // return {false, {}, {}, {}};
      // }
      // const auto this_node = (letter_bits << (26 - letter)).count();

      // const std::size_t prior_nodes = elem_it->preceding_;

      // letters_before = this_node + prior_nodes;
      // ++row_it;
      // if (row_it == rows.end())
      // {
        // next_elem_it = row_end;
      // }
      // else
      // {
        // next_elem_it = std::next(row_it->begin(),
            // static_cast<long>(letters_before));
      // }
    // }

    // //log("Found it!\n");
    // return {true, elem_it, next_elem_it, row_it};
  // }

  // explicit CompactTrieImpl(const Trie& t)
  // {
    // std::vector<const detail::TrieImpl*> q{&t.trie_};
    // std::vector<const detail::TrieImpl*> next_level{};
    // std::vector<std::size_t> levels;

    // compressed_.reserve(t.size() * sizeof(Compressed));
    // std::size_t size = 0;

    // auto safe_uint_add = [](const auto acc, const auto added)
    // {
      // using T = decltype(acc);
      // assert(
          // static_cast<std::uintmax_t>(acc) + static_cast<std::uintmax_t>(added)
          // < std::numeric_limits<T>::max());
      // return static_cast<T>(acc + added);
    // };

    // while (!q.empty())
    // {
      // // fmt::print("Level {}\n", levels.size());
      // const auto level_size = q.size();
      // levels.push_back(compressed_.size());

      // // For item in row
      // using IntType = Compressed::PrecedingType;
      // IntType bits_on = 0;
      // for (const auto& item: q)
      // {
        // // fmt::print("Pushing back {}\n", print_bitset(item->as_compressed()));
        // const std::bitset<27> bits = item->as_compressed();
        // Compressed c{bits, bits_on};
        // compressed_.push_back(c);
        // bits_on = safe_uint_add(bits_on, letters_bitset(c).count());
        // next_level.insert(next_level.end(), item->children_.begin(),
            // item->children_.end());
      // }
      // size += level_size;

      // q = std::move(next_level);
    // }

    // rows_ = make_rows(compressed_, levels);

    // // fmt::print("{}\n", *this);
    // size_ = size;

    // for (const auto& [i, row]: rows_ | ranges::views::enumerate)
    // {
      // std::map<std::size_t, std::size_t> filled_by_n;
      // std::size_t empty_nodes_save_end_of_word = 0;
      // for (const auto& elem: row)
      // {
        // const auto bits = letters_bitset(elem);
        // const auto n = bits.count();
        // ++filled_by_n[n];
        // if (n == 0 && is_end_of_word(elem))
        // {
          // ++empty_nodes_save_end_of_word;
        // }
      // }
    // }

  // }

  // auto size() const
  // {
    // return size_;
  // }

  // auto end() { return compressed_.end(); }

  // [[nodiscard]] bool contains(const std::string& key) const
  // {
    // assert(!key.empty() && "Whether or not searching for the empty string "
        // "makes sense, for now it's unsupported as it's likely a bug");
    // const auto [found, _0, next_it, _1] = find_word(rows_, key);
    // if (found)
    // {
      // return is_end_of_word(*next_it);
    // }
    // return false;
    // // const auto it = this->search(key);
    // // return it != compressed_.end() ? it->test(26) : false;
  // }

  // [[nodiscard]] bool further(const std::string& key) const
  // {
    // assert(!key.empty() && "Whether or not searching for the empty string "
        // "makes sense, for now it's unsupported as it's likely a bug");
    // const auto [found, _0, next_it, _1] = find_word(rows_, key);
    // if (found)
    // {
      // return letters_bitset(*next_it).count() > 0;
    // }
    // return false;
  // }

  // template<class OutputIndexIterator>
  // void contains_and_further(const std::string& stem,
      // const std::string& suffixes,
      // OutputIndexIterator contains_out_it,
      // OutputIndexIterator further_out_it,
      // OutputIndexIterator contains_and_further_out_it) const
  // {
    // // fmt::print("Searching for stem: {} stem and suffixes: {}\n", stem, suffixes);

    // if (rows_.empty()) return;
    // if (stem.empty())
    // {
      // for (const auto [i, c]: suffixes | ranges::views::enumerate)
      // {
        // const std::string s{c};
        // const auto contains = this->contains(s);
        // const auto further = this->further(s);
        // if (contains && further)
        // {
          // *contains_and_further_out_it++ = i;
        // } else if (contains)
        // {
          // *contains_out_it++ = i;
        // } else if (further)
        // {
          // *further_out_it++ = i;
        // }
      // }
      // return;
    // }

    // const auto [found, _, next_it, next_row_it] = find_word(rows_, stem);
    // if (!found)
    // {
      // return;
    // }

    // const auto row_end = (--rows_.end())->end();
    // // fmt::print("Found it at node {}\n", print_bitset(*it));
    // // fmt::print("Found next_it at node {}\n", print_bitset(*next_it));

    // auto follow = [&] (const auto it, const auto row_it, const auto letter_index)
    // {
      // const auto before_node = it->preceding_;
      // // fmt::print("before_node count: {}\n", before_node);
      // const auto before_in_node = (letters_bitset(*it)
          // << (26 - letter_index)).count();
      // // fmt::print("before_in_node count: {}\n", before_in_node);
      // const auto preceding = before_node + before_in_node;
      // // fmt::print("preceding {}\n", preceding);
      // // fmt::print("row_it was at row index: {}\n",
          // // std::distance(rows_.begin(), row_it) );
      // const auto next_row_it = std::next(row_it);
      // // fmt::print("next_row_it at row index: {}\n",
          // // std::distance(rows_.begin(), next_row_it) );
      // // fmt::print("rows_.size() {}\n", rows_.size());
      // if (next_row_it == rows_.end())
      // {
        // // fmt::print("next row == end\n");
        // return row_end;
      // }
      // const auto& row = *next_row_it;
      // const auto final_it = std::next(row.begin(),
          // static_cast<long>(preceding));
      // // fmt::print("Final_it {} \n", print_bitset(*final_it));
      // return final_it;
    // };

    // // const auto stem_bits = letters_bitset(*next_it);
    // // auto next_iter = [](auto&&...) { return void(); };
    // for (const auto [i, c]: suffixes | ranges::views::enumerate)
    // {
      // // fmt::print("\nFor suffix {} finding suffix letter {}\n", i, c);
      // const auto letter_index = static_cast<std::size_t>(c - 97);
      // const auto has_suffix = letters_bitset(*next_it).test(letter_index);
      // bool contains = false;
      // bool further = false;
      // if (has_suffix)
      // {
        // // fmt::print("has_suffix \"{}\" is true, finding last_it\n", c);
        // const auto last_it = follow(next_it, next_row_it, letter_index);
        // if (last_it != row_end)
        // {
          // // fmt::print("last_it: {}\n", print_bitset(*last_it));
          // contains = is_end_of_word(*last_it);
          // further = letters_bitset(*last_it).count();
          // // fmt::print("Contains is end of word: {}\n", contains);
          // // fmt::print("Further on last_it: {}\n", further);
        // }
      // }

      // if (contains && further)
      // {
        // *contains_and_further_out_it++ = i;
      // } else if (contains)
      // {
        // *contains_out_it++ = i;
      // } else if (further)
      // {
        // *further_out_it++ = i;
      // }
      // // fmt::print("contains, further {} {}\n", contains, further);
    // }
  // }

  // static std::string print_bitset(std::bitset<26> b)
  // {
    // std::string s;
    // for (auto i = 0ULL; i < b.size(); ++i)
    // {
      // if (b.test(i)) s.push_back(char(i + 97));
    // }
    // return s;
  // }

  // static std::string print_bitset(std::bitset<27> b)
  // {
    // std::string s;
    // s += "{";
    // for (auto i = 0ULL; i < b.size() - 1; ++i)
    // {
      // if (b.test(i)) s.push_back(char(i + 97));
    // }
    // if (b.test(b.size() - 1))
    // {
      // s += "|";
    // }
    // // os << ' ';
    // s += "} ";
    // return s;
  // }

  // // static std::string print_bitset(std::bitset<27> b)
  // static std::string print_bitset(const Compressed& b)
  // {
    // return print_bitset(b.bits_) + "(" + std::to_string(b.preceding_) + ")";
  // }

  // friend std::ostream& operator<<(std::ostream& os, const CompactTrieImpl& c)
  // {
    // ranges::for_each(c.rows_,
        // [&] (const auto& row)
        // {
          // ranges::for_each(row,
              // [&] (const auto& elem)
              // {
                // os << print_bitset(elem) << ", ";
              // });
          // os << "\n";
        // }
      // );
    // return os;
  // }

  // std::vector<Compressed> compressed_;
  // std::size_t size_;
  // Rows rows_;
// };

// // Nasty wrapper for now to see
// class CompactTrie
// {
  // public:
  // CompactTrie() = default;

  // template<class T>
  // explicit CompactTrie(const T& c)
  // : CompactTrie(std::begin(c), std::end(c))
  // {}

  // template<class T,
    // typename = std::enable_if_t<std::is_convertible_v<
      // T, const std::string_view>>>
  // CompactTrie(std::initializer_list<T> words)
  // : CompactTrie(words.begin(), words.end())
  // {}

  // // This is the constructor that does the work
  // template<class Iterator1, class Iterator2>
  // CompactTrie(Iterator1 first, const Iterator2 last)
  // : trie_(first, last)
  // , compact_trie_(trie_)
  // , to_insert_()
  // {}

  // [[nodiscard]] std::vector<std::string> as_vector() const
  // {
    // return trie_.as_vector();
  // }

  // template<class F>
  // void traverse(F&& f) const
  // {
    // trie_.traverse(std::forward<F>(f));
  // }

  // bool insert(const std::string_view word)
  // {
    // const auto inserted = trie_.insert(word);
    // if (inserted)
    // {
      // to_insert_.push_back(std::string{word});
    // }
    // return inserted;
  // }

  // void rebuild_if_updated() const
  // {
    // if (!to_insert_.empty())
    // {
      // compact_trie_ = CompactTrieImpl{trie_};
      // to_insert_.clear();
    // }
  // }

  // [[nodiscard]] bool contains(const std::string& key) const
  // {
    // rebuild_if_updated();
    // const auto ret = compact_trie_.contains(key);
    // // assert(ret == trie_.contains(key));
    // return ret;
  // }

  // [[nodiscard]] bool further(const std::string& key) const
  // {
    // rebuild_if_updated();
    // // fmt::print("Testing if further has key: \"{}\"\n", key);
    // const auto ret = compact_trie_.further(key);
    // // fmt::print("Testing if further {}, trie: {}, ctrie: {}\n", key,
        // // trie_.further(key), ret);
    // // assert(ret == trie_.further(key));
    // return ret;
  // }

  // template<class OutputIndexIterator>
  // void contains_and_further(const std::string& stem,
      // const std::string& suffixes,
      // OutputIndexIterator contains_out_it,
      // OutputIndexIterator further_out_it,
      // OutputIndexIterator contains_and_further_out_it) const
  // {
    // rebuild_if_updated();
    // compact_trie_.contains_and_further(stem, suffixes,
        // contains_out_it, further_out_it, contains_and_further_out_it);
  // }

  // std::size_t size() const
  // {
    // return trie_.size();
  // }

  // friend std::ostream& operator<<(std::ostream& os, const CompactTrie& ct)
  // {
    // ct.rebuild_if_updated();
    // os << "Trie: " << ct.trie_ << "\n";
    // os << "CompactTrie: " << ct.compact_trie_ << "\n";
    // return os;
  // }

  // Trie trie_;
  // mutable CompactTrieImpl compact_trie_;
  // mutable std::vector<std::string> to_insert_;
// };

// // template Trie::Trie(const std::initializer_list<const char*>);
// // template Trie::Trie(const std::initializer_list<std::string>);
// // template Trie::Trie(const std::initializer_list<std::string_view>);

// // static_assert(std::is_trivially_default_constructible_v<Trie>);
// // static_assert(std::is_nothrow_default_constructible_v<Trie>);
// // static_assert(std::is_nothrow_move_constructible_v<Trie>);
// // static_assert(std::is_nothrow_move_assignable_v<Trie>);

// } // namespace trie

