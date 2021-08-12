Project structure

- dictionary_std_vector
  * Sorted vector of strings. Uses binary searches, so O(log(n)) where n is the number of strings for every operation.
- dictionary_std_set
  * Similar to a sorted vector, but std::set, so red-black tree implementation, with slightly worse performance than the vector, I suspect due to worse cache/memory spatial locality. Also O(log(n)) in number of words in dictionary performance.
- trie
  * Recursive tree structure of nodes, where each node holds a vector-like container of edges, and each edge consists of a character and a pointer to the corresponding child node. 
    To lookup a word of length "m", using a dictionary with "d" distinct characters, for example d == 26 for lowercase ascii and the English alphabet, lookup is O(m * d).
    Realistically, the factor of d will usually be much less than the actual value of d, so really more like just O(m).
    Could say that furthermore, since (in English at least) average word length is much shorter than max(m) anyway, essentially this becomes almost constant time lookup.
- compact_trie
  * Similar to the trie, but now everything's inline, and the whole thing is in one big contiguous memory block.
  Also, this (currently) only works for lowercase ascii, as a bitset<26> is used in a node to indicate the presence/absence of letters.
  We also keep a track of indexes into said vector of nodes that correspond to each row.
  A row corresponds to all the letters at that position in a word.
  Lookup for a word of length "m" is similar to the trie, however now each lookup for every letter of a word, rather than a linear search through a small list of letters, is a lookup for a bit being on in a bitset.
  Then, assuming that letter is present, read off from the node the offset on the next row, where that letter's next node is found.
  Lookup for a word of length m is O(m), plus likely better cache locality (unless change trie to use a pool allocator, which should be very possible as can precompute size from input).
- compact_trie2

- benchmark
  * Google benchmark the time to solve a wordsearch
- cmdline_app
  * Cmdline app used for profiling time taken to solve a wordsearch using a particular solver and dictionary. Uses my slightly modified gperftools profiler.
- gui_app
  * Simple wordsearch gui using ImGui as backend
- solver
  * 
- utility

- cmake
- test

