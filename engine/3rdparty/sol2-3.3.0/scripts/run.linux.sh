#!/usr/bin/env zsh

# # # # sol2
# The MIT License (MIT)
# 
# Copyright (c) 2013-2022 Rapptz, ThePhD, and contributors
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# # This script runs the actual project

echo -e "travis_fold:start:build_preparation\r"
	if [ -z "${SOL2_DIR}" ]
	then
		if [ "${SOL2_CI}" = true ]
		then
			export SOL2_DIR=~/sol2
		else		
			export SOL2_DIR=../..
		fi
	fi

	if [ -z "${SOL2_LUA_VERSION}" ]
	then
		export SOL2_LUA_VERSION=5.4.4
	fi

	if [ -z "${SOL2_PLATFORM}" ]
	then
		export SOL2_PLATFORM=x64
	fi

	mkdir -p build-sol2
	cd build-sol2

	build_dir=$(pwd)

	if [ -f "sol2.compiler.vars" ]
	then
		source ./sol2.compiler.vars
	fi

	if [ ! -z "${CC}" ]
	then
		build_type_cc="-DCMAKE_CXX_COMPILER=${CC}"
	else
		build_type_cc=
	fi

	if [ ! -z "${CXX}" ]
	then
		build_type_cxx="-DCMAKE_CXX_COMPILER=${CXX}"
	else
		build_type_cxx=
	fi

	SOL2_CMAKE_DEFINES=("-DSOL2_LUA_VERSION=${SOL2_LUA_VERSION}")
	SOL2_CMAKE_DEFINES+=("-DSOL2_PLATFORM=${SOL2_PLATFORM}")
	SOL2_CMAKE_DEFINES+=('-DSOL2_CI=ON')
	SOL2_CMAKE_DEFINES+=('-DSOL2_BUILD_LUA=ON')
	SOL2_CMAKE_DEFINES+=('-DBUILD_LUA_AS_DLL=ON')
	SOL2_CMAKE_DEFINES+=('-DSOL2_TESTS=ON')
	SOL2_CMAKE_DEFINES+=('-DSOL2_EXAMPLES=ON')
	SOL2_CMAKE_DEFINES+=('-DSOL2_TESTS_EXAMPLES=ON')
	if [[ ! -z ${SOL2_TEST_SINGLE} ]]
	then
		SOL2_CMAKE_DEFINES+=('-DSOL2_SINGLE=ON')
		SOL2_CMAKE_DEFINES+=('-DSOL2_TESTS_SINGLE=ON')
		SOL2_CMAKE_DEFINES+=('-DSOL2_EXAMPLES_SINGLE=ON')
	fi
	if [[ ! -z ${SOL2_TEST_INTEROP} ]]
	then
		SOL2_CMAKE_DEFINES+=('-DSOL2_INTEROP_EXAMPLES=ON')
		SOL2_CMAKE_DEFINES+=('-DSOL2_TESTS_INTEROP_EXAMPLES=ON')
		SOL2_CMAKE_DEFINES+=('-DSOL2_DYNAMIC_LOADING_EXAMPLES=ON')
		SOL2_CMAKE_DEFINES+=('-DSOL2_TESTS_DYNAMIC_LOADING_EXAMPLES=ON')	
		if [[ ! -z ${SOL2_TEST_SINGLE} ]]
		then
			SOL2_CMAKE_DEFINES+=('-DSOL2_INTEROP_EXAMPLES_SINGLE=ON')
			SOL2_CMAKE_DEFINES+=('-DSOL2_DYNAMIC_LOADING_EXAMPLES_SINGLE=ON')
		fi
	fi

	mkdir -p Debug Release

	echo "=== Compiler and tool variables ==="
	ninja --version
	cmake --version
	echo sol2 source dir : "${SOL2_DIR}"
	echo build_type_cc   : "${build_type_cc}"
	echo build_type_cxx  : "${build_type_cxx}"
	echo cmake defines   : "${SOL2_CMAKE_DEFINES[@]}"
echo -e "travis_fold:end:build_preparation\r"

echo -e "travis_fold:start:build.debug\r"
	cd Debug
	cmake "${SOL2_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Debug ${build_type_cc} ${build_type_cxx} "${SOL2_CMAKE_DEFINES[@]}"
	cmake --build . --config Debug
echo -e "travis_fold:end:build.debug\r"
echo -e "travis_fold:start:test.debug\r"
	ctest --build-config Debug --output-on-failure
	cd ..
echo -e "travis_fold:end:test.debug\r"

echo -e "travis_fold:start:build.release\r"
	cd Release
	cmake "${SOL2_DIR}" -G Ninja -DCMAKE_BUILD_TYPE=Release ${build_type_cc} ${build_type_cxx} "${SOL2_CMAKE_DEFINES[@]}"
	cmake --build . --config Release
echo -e "travis_fold:end:build.release\r"
echo -e "travis_fold:start:test.release\r"
	ctest --build-config Release --output-on-failure
	cd ..
echo -e "travis_fold:end:test.release\r"
