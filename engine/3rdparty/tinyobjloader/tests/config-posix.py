exe = "tester"

# "gnu" or "clang"
toolchain = "gnu"

# optional
link_pool_depth = 1

# optional
builddir = {
    "gnu" :  "build"
  , "msvc" :  "build"
  , "clang" :  "build"
    }

includes = {
    "gnu" : [ "-I." ]
  , "msvc" : [ "/I." ]
  , "clang" : [ "-I." ]
    }

defines = {
    "gnu" : [ ]
  , "msvc" : [ ]
  , "clang" : [  ]
    }

cflags = {
    "gnu" : [ "-O2", "-g" ]
  , "msvc" : [ "/O2" ]
  , "clang" : [ "-O2", "-g" ]
    }

# Warn as much as possible: http://qiita.com/MitsutakaTakeda/items/6b9966f890cc9b944d75
cxxflags = {
    "gnu" : [ "-O2", "-g", "-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused", "-fsanitize=address" ]
  , "msvc" : [ "/O2", "/W4" ]
  , "clang" : [ "-O2", "-g", "-Werror -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic", "-fsanitize=address" ]
    }

ldflags = {
    "gnu" : [ "-fsanitize=address" ]
  , "msvc" : [ ]
  , "clang" : [ "-fsanitize=address" ]
    }

cxx_files = [ "tester.cc" ]
c_files = [ ]

# You can register your own toolchain through register_toolchain function
def register_toolchain(ninja):
    pass

