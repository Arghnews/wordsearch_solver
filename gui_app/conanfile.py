import os

from conans import ConanFile, CMake, tools

class TestProjectConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake_find_package"
    requires = ("fmt/[>=7.0.2]", "imgui_my/[>=0.2.3]@arghnews/testing",
            "wordsearch_solver/0.1.5", "gperftools/0.1.1@arghnews/testing")

    def build(self):
        cmake = CMake(self)
        # Current dir is "test_package/build/<build_id>" and CMakeLists.txt is
        # in "test_package"
        cmake.configure()
        cmake.build()

    def configure(self):
        tools.check_min_cppstd(self, "17")

    def test(self):
        if not tools.cross_building(self.settings):
            #  os.chdir("../bin")
            self.run(".%sexample" % os.sep)
