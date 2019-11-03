#ifndef WORDSEARCH_SOLVER_DEFS_H
#define WORDSEARCH_SOLVER_DEFS_H

#include <cstddef>
#include <memory>
#include <string>
#include <set>
#include <utility>
#include <vector>

namespace wordsearch_solver
{
// TODO: move these to an appropriate place. (Circular includes problem)
using Grid = std::shared_ptr<std::vector<std::string>>;
//using Dictionary = std::vector<std::string>;
//using Dictionary = std::set<std::string>;
using Index = std::pair<std::size_t, std::size_t>;
using Indexes = std::vector<Index>;
}

#endif // WORDSEARCH_SOLVER_DEFS_H
