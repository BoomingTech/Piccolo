exe = "tester.exe"

toolchain = "msvc"

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
    "gnu" : [ "-DEXAMPLE=1" ]
  , "msvc" : [ "/DEXAMPLE=1" ]
  , "clang" : [ "-DEXAMPLE=1" ]
    }

cflags = {
    "gnu" : [ "-O2", "-g" ]
  , "msvc" : [ "/O2" ]
  , "clang" : [ "-O2", "-g" ]
    }

cxxflags = {
    "gnu" : [ "-O2", "-g" ]
  , "msvc" : [ "/O2", "/W4", "/EHsc"]
  , "clang" : [ "-O2", "-g", "-fsanitize=address" ]
    }

ldflags = {
    "gnu" : [ ]
  , "msvc" : [ ]
  , "clang" : [ "-fsanitize=address" ]
    }

# optionsl
cxx_files = [ "tester.cc" ]
c_files = [ ]

# You can register your own toolchain through register_toolchain function
def register_toolchain(ninja):
    pass

