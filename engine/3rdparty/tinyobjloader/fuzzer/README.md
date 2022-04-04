# Fuzzing test

Do fuzzing test for tinyobjloader

## Supported API

* [x] ParseFromString

## Requirements

* clang with fuzzer support(`-fsanitize=fuzzer`. at least clang 8.0 should work)

## Setup

### Ubuntu 18.04

```
$ sudo apt install clang++-8
$ sudo apt install libfuzzer-8-dev
```

Optionally, if you didn't set `update-alternatives` you can set `clang++` to point to `clang++8`

```
$ sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-8 10
$ sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-8 10
```

## How to compile

Fuzz target is compiled with the rest of the project when environment variable `LIB_FUZZING_ENGINE` is defined when running cmake
With clang, you can compile with 
```
$ export LIB_FUZZING_ENGINE=-fsanitize=fuzzer
$ mkdir build && cd build
$ cmake .. -DBUILD_SHARED_LIBS=OFF
$ make -j $(nproc)
```

## How to run

Increase memory limit. e.g. `-rss_limit_mb=2000`
cf libfuzzer.info for all options

```
$ ./fuzz_ParseFromString -rss_limit_mb=2000
```

## Regression tests

See `regression_runner/`
