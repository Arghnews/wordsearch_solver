#ifndef SOLVER_TPP
#define SOLVER_TPP

#include "wordsearch_solver/solver/solver.hpp"
#include "matrix2d/matrix2d.hpp"

// #ifndef __EMSCRIPTEN__
// #include <gperftools/profiler.h>
// #endif

#include <boost/container/static_vector.hpp>
// #include "static_vector/experimental/fixed_capacity_vector.hpp"
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <range/v3/action/sort.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/contains.hpp>
#include <range/v3/algorithm/copy.hpp>
#include <range/v3/algorithm/equal.hpp>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/range/primitives.hpp> // ranges::size
#include <range/v3/view/enumerate.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <ostream>
#include <range/v3/view/zip.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

// TODO: test on 1x1 grid
// and 1x5
// and 0x0
// and more
// Consider/test a class that wraps/holds a container of Index s and a string
// for tail and suffixes, and then inherit with Matrix2d one? Wow actual
// inheritance, my god, or just compose really?

namespace solver
{

template <class T, std::size_t N>
// using static_vector = std::experimental::fixed_capacity_vector<T, N>;
// llvm_small_vector
using static_vector = boost::container::static_vector<T, N>;

template <class SolverDict>
// std::unordered_map<std::string,
// std::vector<std::vector<matrix2d::Index>>> Would like to express this
// returns something with an interface like the above type. Perhaps just into
// OutputIterator template type to avoid this altogether? (Need co_yield
// really).
void solve_index(const SolverDict &solver_dict, const WordsearchGrid &grid,
                 const Index start_index,
                 WordToListOfListsOfIndexes &word_to_list_of_indexes) {
  // Coroutines in cppcoro needs libc++, ballache
  // Folly coroutines unclear if need it
  // Going to try boost coroutine2 for now at least as already installed

  // Unsure if necessary
  if (grid.empty()) {
    return;
  }

  if (start_index.y > grid.rows() || start_index.x > grid.columns()) {
    throw std::runtime_error("Start index out of range of wordsearch grid");
  }

#define LOG(...)
// #define LOG fmt::print

  const auto rows = grid.rows();
  const auto cols = grid.columns();

  // using boost::container::static_vector;

  LOG("Grid {}\n", grid);
  LOG("rows x cols = {} * {}\n", rows, cols);
  LOG("Start index: {}", start_index);

  const auto index_to_char = [&grid] (const auto index) {
    return grid(index);
  };

  std::vector<static_vector<Index, 8>> q;

  static_vector<Index, 8> suffixes{start_index};
  std::string suffixes_string{index_to_char(start_index)};

  Tail tail;
  std::string tail_string;
  matrix2d::Matrix2d<bool> tail_matrix{grid.rows(), grid.columns()};

  const auto assert_invariants = [&]() {
#if 0
    // These checks are valid, but seriously slow. They make builds of the
    // whole library orders of magnitude slower when enabled. For now disable,
    // should find a neat way to enable via preprocessor defines - how to make
    // this work with conan?
    const auto q_fronts = ranges::views::transform(q, [](const auto &indexes) {
      assert(!indexes.empty());
      return indexes.front();
    });
    const auto q_fronts_string =
        ranges::views::transform(q_fronts, index_to_char);
    assert(ranges::all_of(
        q, [](const auto &indexes) { return !indexes.empty(); }));
    assert(ranges::equal(ranges::views::transform(suffixes, index_to_char),
                         suffixes_string));

    assert(ranges::equal(ranges::views::transform(tail, index_to_char),
                         tail_string));

    const auto rows = tail_matrix.rows_iter();
    for (const auto &[i, row] : ranges::views::enumerate(rows)) {
      for (const auto j : ranges::views::ints(0UL, row.size())) {
        assert(ranges::contains(tail, Index{i, j}) == tail_matrix(i, j));
        (void)j;
      }
    }
#endif
  };

  while (true) {

    assert_invariants();

    LOG("\nq: {}\n", q);

    LOG("Tail: {}\n", tail);
    LOG("tail_string: {}\n", tail_string);
    LOG("Suffixes: {}\n", suffixes);
    LOG("suffixes_string: {}\n", suffixes_string);

    static_vector<std::pair<bool, bool>, 8> contains_further;

    // const auto contains_further_start_time = now();
    // ProfilerDisable();
    solver_dict.contains_further(tail_string, suffixes_string,
                                 std::back_inserter(contains_further));
    // ProfilerEnable();
    // time_spent_in_contains_further += now() - contains_further_start_time;

    LOG("contains_further: {}\n", contains_further);

    assert(suffixes.size() == suffixes_string.size());
    assert(suffixes.size() == contains_further.size());

    static_vector<Index, 8> next_layer;
    for (const auto i: ranges::views::ints(0UL, suffixes.size())) {
      const auto [contains, further] = contains_further[i];
      LOG("For index in suffixes {}: {}/{}\n", i, suffixes[i], suffixes_string[i]);
      LOG("contains, further {} {}\n", contains, further);
      if (contains) {
        // Note, potential optim here. If can lookup by temporarily adding to
        // tail, avoid allocation in case where key already exists in map.
        LOG("Outputing word, indexes: {}, {}\n", word, indexes);
        // Waiting for co_yield to output neatly from here...

        assert_invariants();

        const auto suffix = suffixes[i];
        const auto suffix_char = suffixes_string[i];

        auto indexes = tail;
        indexes.push_back(suffix);

        word_to_list_of_indexes[tail_string + suffix_char].emplace_back(
            std::move(indexes));

        assert_invariants();
      }
      if (further) {
        LOG("Adding to next_layer {}\n", suffixes[i]);
        next_layer.push_back(suffixes[i]);
      }
    }

    if (!next_layer.empty()) {
      LOG("Appending to q next_layer: {}\n", next_layer);
      q.push_back(next_layer);
      tail.push_back(next_layer.front());
      tail_string.push_back(index_to_char(next_layer.front()));
      tail_matrix(next_layer.front()) = true;
    }
    else {
      for (; !q.empty() && q.back().size() <= 1;)
      {
        ////assert_invariants();
        LOG("Popping from back of q: {}\n", q.back());
        q.pop_back();
        const auto index = tail.back();
        tail.pop_back();
        tail_string.pop_back();
        tail_matrix(index) = false;
        ////assert_invariants();
      }

      if (!q.empty()) {
        // Assertion is true else we'd have popped it in while loop above
        ////assert_invariants();
        LOG("Removing from front of back of q: {}\n", q.back().front());
        //assert(q.back().size() >= 2);

        const auto index_to_remove = q.back()[0];
        const auto index_to_add = q.back()[1];

        q.back().erase(q.back().begin()); // Pop front
        tail.back() = index_to_add;
        tail_string.back() = index_to_char(index_to_add);
        tail_matrix(index_to_remove) = false;
        tail_matrix(index_to_add) = true;

        assert_invariants();
      }

    }

    // Loop termination condition
    if (q.empty()) {
      break;
    }

    assert_invariants();

    assert(!q.back().empty());
    assert(cols > 0);
    assert(grid.size() > 1); // must be at least bigger than 1 x 1 square
    const auto n = q.back().front();
    auto has = [&tail_matrix](const auto y, const auto x) {
      return tail_matrix(y, x);
    };

    // TODO: move this to function, and then into loop iteration
    // Avoid using minus operations on x and y as they are unsigned and this
    // will underflow
    // Order in which we access adjacent values, eg. SW = SouthWest = {+1, -1}
    // NW, N, NE, W, E, SW, S, SE

    suffixes.clear();
    suffixes_string.clear();
    if (n.y > 0 && n.x > 0 && !has(n.y - 1, n.x - 1)) {
      suffixes.emplace_back(n.y - 1, n.x - 1);
      suffixes_string.push_back(index_to_char(Index{n.y - 1, n.x - 1}));
    }
    if (n.y > 0 && !has(n.y - 1, n.x)) {
      suffixes.emplace_back(n.y - 1, n.x);
      suffixes_string.push_back(index_to_char(Index{n.y - 1, n.x}));
    }
    if (n.y > 0 && n.x + 1 < cols && !has(n.y - 1, n.x + 1)) {
      suffixes.emplace_back(n.y - 1, n.x + 1);
      suffixes_string.push_back(index_to_char(Index{n.y - 1, n.x + 1}));
    }
    if (n.x > 0 && !has(n.y, n.x - 1)) {
      suffixes.emplace_back(n.y, n.x - 1);
      suffixes_string.push_back(index_to_char(Index{n.y, n.x - 1}));
    }
    if (n.x + 1 < cols && !has(n.y, n.x + 1)) {
      suffixes.emplace_back(n.y, n.x + 1);
      suffixes_string.push_back(index_to_char(Index{n.y, n.x + 1}));
    }
    if (n.y + 1 < rows && n.x > 0 && !has(n.y + 1, n.x - 1)) {
      suffixes.emplace_back(n.y + 1, n.x - 1);
      suffixes_string.push_back(index_to_char(Index{n.y + 1, n.x - 1}));
    }
    if (n.y + 1 < rows && !has(n.y + 1, n.x)) {
      suffixes.emplace_back(n.y + 1, n.x);
      suffixes_string.push_back(index_to_char(Index{n.y + 1, n.x}));
    }
    if (n.y + 1 < rows && n.x + 1 < cols && !has(n.y + 1, n.x + 1)) {
      suffixes.emplace_back(n.y + 1, n.x + 1);
      suffixes_string.push_back(index_to_char(Index{n.y + 1, n.x + 1}));
    }
  }
#undef LOG
}

template <class SolverDict>
WordToListOfListsOfIndexes solve(const SolverDict &solver_dict,
                                 const WordsearchGrid &grid) {
  WordToListOfListsOfIndexes word_to_list_of_indexes;

  const auto rows = grid.rows_iter();
  for (const auto& [i, row]: ranges::views::enumerate(rows)) {
    for (const auto [j, elem]: ranges::views::enumerate(row)) {
      // fmt::print("Processing: {}, {}\n", i, j);
      solve_index(solver_dict, grid, Index{i, j}, word_to_list_of_indexes);
    }
  }
  return word_to_list_of_indexes;
}
}

#endif // SOLVER_TPP
