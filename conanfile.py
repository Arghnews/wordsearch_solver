from conans import ConanFile, CMake, tools

import pathlib
import re

from distutils import dir_util

class WordsearchsolverConan(ConanFile):
    name = "wordsearch_solver"
    version = "0.1.8"
    license = "MIT"
    author = "Justin Riddell arghnews@hotmail.co.uk"
    url = "https://github.com/Arghnews/wordsearch_solver"
    description = "Wordsearch solver"
    topics = ("c++", "wordsearch")
    settings = "os", "compiler", "build_type", "arch"

    options = {"shared": [True, False],
            "trie": [True, False],
            "compact_trie": [True, False],
            "compact_trie2": [True, False],
            "dictionary_std_set": [True, False],
            "dictionary_std_vector": [True, False],
            }
    default_options = {"shared": False,
            "trie": True,
            "compact_trie": True,
            "compact_trie2": True,
            "dictionary_std_set": True,
            "dictionary_std_vector": True,
            }

    generators = "cmake_find_package"

    #  requires = "fmt/6.2.1"
    requires = ("fmt/[>=7.0.1]", "range-v3/[>=0.11.0]",
            #  "google-profiler/0.1",
            #  "boost_container/[>=1.69.0]@bincrafters/stable",
            #  "matrix2d/[>=0.2.5]@justin_riddell/stable",
            "matrix2d/[>=0.2.6]@arghnews/testing",
            #  "static_vector/0.1",
            #  "solver_utility/0.1",
            "boost/[>=1.76.0]",
            #  "boost_iterator/[>=1.69.0]@bincrafters/stable",
            "catch2/[>=2.13.0]",
            "benchmark/[>=1.5.3]",
            #  "args-parser/[>=6.1.1]", # Pretty crap
            "cxxopts/[>=2.2.1]",
            "gperftools/[>=0.1.1]@arghnews/testing",
            #  "boost_container/1.69.0@bincrafters/stable",
            #  "llvm_small_vector/0.1",
            )

    _dict_impls = ["trie", "compact_trie", "compact_trie2",
            "dictionary_std_set", "dictionary_std_vector",
            ]

    def export_sources(self):
        self.copy("*", src=".", dst="source_subfolder")
        pass

    #  def source(self):
        #  git = tools.Git(folder="source_subfolder")
        #  git.clone("https://github.com/Arghnews/wordsearch_solver", shallow=True)
        #  #  dir_util.copy_tree("/home/justin/cpp/ws3", "source_subfolder")

    def build(self):
        cmake = CMake(self)
        version_cmake = pull_version_number("source_subfolder/version.cmake",
                self.name)
        self_version = self.version
        assert version_cmake == self_version, \
                f"version.cmake {version_cmake} != self.version {self_version}"

        assert all(d in self.options for d in self._dict_impls)
        cmake.definitions["WORDSEARCH_SOLVERS"] = ";".join(dict_impl
                for dict_impl in self._dict_impls
                if self.options.get_safe(dict_impl))
        cmake.configure(source_folder="source_subfolder")
        cmake.build()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        # Check version numbers in version.cmake and self match
        # Not sure in which conan method this check should really be
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
        # https://stackoverflow.com/a/409470/8594193
        # Dumbass g++ linking order, static lib solver depends on implementation
        # libs, so solver must be first
        self.cpp_info.libs.remove("solver")
        self.cpp_info.libs.insert(0, "solver")
        #  print("The libs: ", self.cpp_info.libs)

assert all(d in WordsearchsolverConan.options
        for d in WordsearchsolverConan._dict_impls)

def pull_version_number(filepath, project_name):
    version_file = pathlib.Path(filepath).read_text()
    project_name = project_name.upper()

    def get_component(version_part):
        pattern = "set\({}_{} ([0-9]+)\)".format(project_name, version_part)
        p = re.search(pattern, version_file)
        assert p
        return p.group(1)

    major = get_component("MAJOR")
    minor = get_component("MINOR")
    patch = get_component("PATCH")
    return ".".join((major, minor, patch))

