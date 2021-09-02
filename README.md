Inspired by the struggle for high scores in [spelltower](http://spelltower.com/), this program solves wordsearches.

Example program, as in example/main.cpp

```cpp
#include "wordsearch_solver/wordsearch_solver.hpp"

#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

int main() {
  // Define a wordsearch
  const std::vector<std::string> wordsearch = {
    "adg",
    "beh",
    "cfi",
  };

  // Define a container of string-like objects that is the dictionary
  const std::initializer_list<std::string_view> dictionary = {"zoo", "badge",
                                                              "be", "beg"};
  // const auto dictionary = utility::read_file_as_lines("path/to/dict");

  const auto solver = solver::SolverDictFactory{}.make("trie", dictionary);
  const auto grid = solver::make_grid(wordsearch);

  // Solve the wordsearch
  const solver::WordToListOfListsOfIndexes answer = solver::solve(solver, grid);

  for (const auto& [word, indexes] : answer) {
    fmt::print("{}: {}\n", word, indexes);
  }
  // [0, 0] is top left, 'a'. [1, 0] is 'b'. [2, 1] is 'f'.
  // Prints:
  // beg: {{[1, 0], [1, 1], [0, 2]}}
  // be: {{[1, 0], [1, 1]}}
  // badge: {{[1, 0], [0, 0], [0, 1], [0, 2], [1, 1]}}
}
```

NOTE: must `cd` to build/test and run ./the_test from this dir!

NOTE: requires gnu-gold linker to build with gcc, and lld (llvm linker) to build with clang

Built and tested with gcc8, clang 10 and clang 12

## How to build

The normal [conan](https://conan.io/) + [CMake](https://cmake.org/) build process
Essentially what's in make.sh

```bash
# From project root
mkdir build
cd build
conan install .. # May add "-s build_type=Release"
cmake -DCMAKE_BUILD_TYPE=Release .. # Or "Debug" or "RelWithDebInfo"
cmake --build .
# cmake --build . -t install # If installing
```

## Alternative usage

[jfrog artifactory](https://jfrog.com/artifactory/) hosts conan packages for free

Add my repo with conan, calling it whatever you wish, then consume via conan as normal

```bash
conan remote add arghnews_jfrog https://arghnews.jfrog.io/artifactory/api/conan/arghnews-conan
```

Then add to conanfile.txt or conanfile.py, in requires:

```bash
wordsearch_solver/[>=0.1.10]@arghnews/testing # Or whatever version, or version range
```

And build/use normally your project normally with conan

# Project structure

- dictionary_std_vector

Sorted vector of strings. Uses binary searches, so O(log(n)) where n is the number of strings for every operation.

---

- dictionary_std_set

Similar to a sorted vector, but std::set, so red-black tree implementation, with slightly worse performance than the vector, I suspect due to worse cache/memory spatial locality. Also O(log(n)) in number of words in dictionary performance.

---

- trie

Recursive tree structure of nodes, where each node holds a vector-like container of edges, and each edge consists of a character and a pointer to the corresponding child node. 
To lookup a word of length "m", using a dictionary with "d" distinct characters, for example d == 26 for lowercase ascii and the English alphabet, lookup is O(m * d).
Realistically, the factor of d will usually be much less than the actual value of d, so really more like just O(m).
Could say that furthermore, since (in English at least) average word length is much shorter than max(m) anyway, essentially this becomes almost constant time lookup.

---

- compact_trie

Similar to the trie, but now everything's inline, and the whole thing is in one big contiguous memory block.
NOTE: this (currently) only works for lowercase ascii.
A node consists of a bitset of size 26 (bits), with each bit representing an edge to a child node and a letter. A node also has a bool to indicate whether it's a word end, and an int to indicate how many nodes before this one on the same row existed, which is required for calculating the offset in the next row of the child nodes.
We also keep a track of indexes into said vector of nodes that correspond to each row.
A row corresponds to all the letters at that position in a word.
Lookup for a word of length "m" is similar to the trie, however now each lookup for every letter of a word, rather than a linear search through a small list of letters, is a lookup for a bit being on in a bitset.
Then, assuming that letter is present, read off from the node the offset on the next row, where that letter's next node is found.
Lookup for a word of length m is O(m), plus likely better cache locality (unless change trie to use a pool allocator, which should be very possible as can precompute size from input).

---

- compact_trie2

The idea with this trie was to have everything inline in one data structure.
Unfortunately, performance tends to be only slightly better than the (pointer) trie, and worse than the compact_trie. It has to do more work at each node to calculate the offset to the next one.

```
Example layout for a node that has two children
Node size is byte at A (size) * 3 + 2.
|____| = 1 byte

                        'a'   'c'
|____||____||____||____||____||____||____||____||____||____|
 A     B     C     DE    F     G     H     H     I     I    

- A: 1 byte, size, number of characters in this node
- B, C, D: Bytes B and C plus all but the highest bit in D make up the offset,
	from the start of the next row, where this node's children begin.
- E: Highest bit in byte D. Flag to signal whether this node represents
	the end of a word
- F, G: Sorted 1 byte entries of child nodes.
	Ie. F may be the ascii corresponding to 'a', G == 'c'
- H, I: Corresponding child intra-node offsets, from the next row node offset
	given by BCD (with highest bit zeroed).
Ie. to find the child node from the example node for the letter 'c', get the
	position of the start of the next row,
	add BCD (without highest bit) and add the letter offset in the 2 byte int I.
```

---

- benchmark

Google benchmark the time to solve a wordsearch

My benchmark I use, in bench.sh

Uses the dictionary file that is ~115k lines, and a 100x100 wordsearch, measures the time to solve it

This benchmark was run using clang 12, libstdc++8 and an SSD (Crucial MX500).
LTO was used.
|Benchmark                                                       |     Time |            CPU |  Iterations
|----------------------------------------------------------------|----------|----------------|------------
|bench_long_words/trie::Trie                                     |   109 ms |         109 ms |           6
|bench_long_words/compact_trie::CompactTrie                      |  74.3 ms |        74.3 ms |           9
|bench_long_words/compact_trie2::CompactTrie2                    |  92.1 ms |        92.1 ms |           8
|bench_long_words/dictionary_std_vector::DictionaryStdVector     |   330 ms |         330 ms |           2
|bench_long_words/dictionary_std_set::DictionaryStdSet           |   601 ms |         601 ms |           1

Some points to take away:
* Unsurprising that the dictionary_std_vector/dictionary_std_set solvers are slowest. They simple use sorted std containers, so their furhter functions will return true too often (as they can't do better) and both their contains and further functions are O(log(n)) where n is the size of the dictionary.

When run with gcc8 instead of clang12, see much worse performance for the compact tries.

|Benchmark                                                       |    Time  |           CPU  | Iterations
|----------------------------------------------------------------|----------|----------------|-----------
|bench_long_words/trie::Trie                                     |  109 ms  |        109 ms  |          6
|bench_long_words/compact_trie::CompactTrie                      |  127 ms  |        127 ms  |          6
|bench_long_words/compact_trie2::CompactTrie2                    |  103 ms  |        103 ms  |          7
|bench_long_words/dictionary_std_vector::DictionaryStdVector     |  360 ms  |        360 ms  |          2
|bench_long_words/dictionary_std_set::DictionaryStdSet           |  689 ms  |        689 ms  |          1

---

- cmdline_app

Cmdline app used for profiling time taken to solve a wordsearch using a particular solver and dictionary. Uses my slightly modified gperftools profiler.

---

- gui_app

Simple wordsearch gui using ImGui as backend

---

- solver

Contains the algorithm to actually solver a wordsearch.
Exposes types that clients should use to consume this library.

---

- utility

---

- cmake

---

- test


