from conans import ConanFile, CMake, tools

import os
import pathlib
import re
import shutil

from distutils import dir_util
import distutils

class WordsearchsolverConan(ConanFile):
    name = "wordsearch_solver"
    version = "0.1.5"
    license = "MIT"
    author = "Justin Riddell arghnews@hotmail.co.uk"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of Wordsearchsolver here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")
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
            "args-parser/[>=6.1.1]",
            "gperftools/[>=0.1.1]@arghnews/testing",
            #  "boost_container/1.69.0@bincrafters/stable",
            #  "llvm_small_vector/0.1",
            )

    _dict_impls = ["trie", "compact_trie", "compact_trie2",
            "dictionary_std_set", "dictionary_std_vector",
            ]

    _cmake = None


    def source(self):
        #  self.run("git clone https://github.com/conan-io/hello.git")
        # This small hack might be useful to guarantee proper /MT /MD linkage
        # in MSVC if the packaged project doesn't have variables to set it
        # properly
        #  self.copy("*", src = "/home/justin/cpp/ws", keep_path = True)
        # https://stackoverflow.com/a/1994840/8594193
        #  shutil.copytree("/home/justin/cpp/ws", "source_subfolder")
        dir_util.copy_tree("/home/justin/cpp/ws3", "sauce")
        #  def source_folder(self):
        #  return "source_subfolder"

    #  def configure_cmake(self):
        #  if self._cmake:
            #  return self._cmake
        #  self._cmake = CMake(self)
        #  #  self._cmake.configure(source_folder = "source")
        #  self._cmake.configure(source_folder = "sauce")
        #  return self._cmake

    def build(self):
        cmake = CMake(self)

        assert all(d in self.options for d in self._dict_impls)
        #  print(self.options)
        #  for d in self._dict_impls:
            #  print(d)
            #  print(self.options[d])
        cmake.definitions["WORDSEARCH_SOLVERS"] = ";".join(dict_impl
                for dict_impl in self._dict_impls
                if self.options.get_safe(dict_impl))
        #  print("WS: ", cmake.definitions["WORDSEARCH_SOLVERS"])
        cmake.configure(source_folder = "sauce")
        cmake.build()
        cmake.test()
        # Explicit way:
        # self.run('cmake %s/hello %s'
        #          % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        #  cmake = self.configure_cmake()
        cmake = CMake(self)
        # Check version numbers in version.cmake and self match
        # Not sure in which conan method this check should really be
        import os
        print(os.getcwd())
        assert pull_version_number("sauce/version.cmake", self.name) == self.version
        cmake.install()
        #  print("yam")
        #  print(self)
        #  print(type(self))
        #  cmake = CMake(self)
        #  print("moo")
        #  cmake.install()
        #  self.copy("*.h", dst="include", src="hello")
        #  self.copy("*hello.lib", dst="lib", keep_path=False)
        #  self.copy("*.dll", dst="bin", keep_path=False)
        #  self.copy("*.so", dst="lib", keep_path=False)
        #  self.copy("*.dylib", dst="lib", keep_path=False)
        #  self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        # TODO: implement this if want to be able to use library from not cmake
        self.cpp_info.libs = tools.collect_libs(self)

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

