#ifndef WORDSEARCH_SOLVER_DEFS_H
#define WORDSEARCH_SOLVER_DEFS_H

#include <cstddef>
#include <memory>
#include <string>
#include <set>
#include <utility>
#include <vector>
#include <variant>

namespace wordsearch_solver
{
// TODO: move these to an appropriate place. (Circular includes problem)
using Grid = std::shared_ptr<std::vector<std::string>>;
//using Dictionary = std::vector<std::string>;
//using Dictionary = std::set<std::string>;
using Index = std::pair<std::size_t, std::size_t>;
using Indexes = std::vector<Index>;

template<class... Solvers>
struct Solver
{
  template<class OutputIndexIterator>
  void contains_and_further(
      const std::string& stem,
      const std::string& suffixes,
      OutputIndexIterator contains,
      OutputIndexIterator further,
      OutputIndexIterator contains_and_further
      ) const
  {
    return std::visit([&](auto&& solver)
        {
          return solver.contains_and_further(
              stem,
              suffixes,
              contains,
              further,
              contains_and_further);
        }, solvers_);
  }
  std::variant<Solvers...> solvers_;
};

// TODO: change this impl
struct Result
{
  std::vector<std::size_t> contains;
  std::vector<std::size_t> further;
  std::vector<std::size_t> contains_and_further;
  void clear()
  {
    contains.resize(0);
    further.resize(0);
    contains_and_further.resize(0);
  }
};

}

#endif // WORDSEARCH_SOLVER_DEFS_H
