- wordsearch_solver:: namespace around everything

✓ Remove duplication of version in CMakeLists.txt and conanfile.py
Add git hooks type check to ensure this

✓ Better way to add solvers than the horrendous #ifdefs in cmdline_app/main.cpp

- Implement copy construction for solvers, so can use benchmark::RegisterBenchmark as needs copy construction. Then redo benchmark to clean it up as a result

- Make more stuff private, that should be

- Run clang format on everything

- Couldn't think where to put this, didn't want to lose it.
  It turns out, if you consume a conan package like with my matrix2d one but you use the cmake generator, not the cmake_find_package,
  then you'll get hit with warnings that you don't get if consuming via find_package.
  Take for example this, built with
  clang++ -std=c++17 -Wconversion a.cpp

```cpp
// a.cpp
#include "boost/container/small_vector.hpp"
int main() {
  boost::container::small_vector<int, 4> v;
  return v.cend() > v.cbegin();
}
```

Complains about boost/container/vector.hpp, line 1458 in cend() const
Need to change
```cpp
it += this->m_holder.m_size;
```
to
```cpp
it += static_cast<difference_type>(this->m_holder.m_size);
```

boost::container::small_vector is used in matrix2d which is used by this project.
I only, however, get this warning when compiling using conan and the cmake generator (not the cmake find_package one) and clang.
gcc gives different -Wconversion related warnings from other parts of boost.

Solutions/attempts:
- Do nothing. Pro: preserves my sanity. Just consume via conan and the generator "cmake_find_package", not "cmake"
- Have tried stuff like [this](https://stackoverflow.com/q/52135983/8594193), as I know cmake has target_include_directories(SYSTEM) which corresponds to the -isystem flag, but struggling to find how to make it work for libraries linked except with stuff like that
- In the same vein, some cmake-foo to move the include dirs from the target proeprty INTERFACE_INCLUDE_DIRECTORIES to SYSTEM_INTERFACE_INCLUDE_DIRECTORIES, not sure if possible
- Could just shove the pragma to say it's a system header and be done with it
- "Pragma push diagnostic ignored -Wno-conversion" or whatever the hell the line is
Is boost *meant* to only be included as a system dependency?

This took a while to find.
Was getting mysterious undefined reference to main style errors when using conan create only in Release builds, not Debug, in the test files, using gcc.
Long story short, in Release mode in the main CMakeLists.txt I had specified -flto in only the link lines for Debug, but for the link and compile for Release.
It turns out -flto was presumably allowing the linker to throw away the main function, generated either by boost test internal boost stuff, or then by Catch2 in my testing
as defined by CATCH_CONFIG_MAIN, or even my own int main().
Eventually even compiling hello world with `g++ -flto a.cpp` failed.
That's when I remembered, in my infinite wisdom, a while back, I'd decided that rather than writing "-fuse-ld=lld" every time in my link options, I'd see what happened if I symlinked /usr/bin/ld (which is symlinked to the slow old bfd linker) to ld.lld (llvm's faster, better linker). And all was well. Except that gcc needs special handling - awareness from the linker - to handle its lto objects, at least without -ffat-lto-objects.
Release, linking with default bfd, 1m43s, gold, 1m30s, clang+lld, 30s


✓ Currently have some circular includes in solver, fix those

- Clean up the constructor delegation/initialisation mess between compact_trie and compact_trie2
    * Thinking move something out to utility

