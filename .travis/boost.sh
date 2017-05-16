#!/bin/bash
set -x
mkdir boost
pushd boost
wget -nc https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.gz
tar -zxf boost_1_64_0.tar.gz
pushd boost_1_64_0
./bootstrap.sh
if [ ${1} = "clang" ]; then PROPERTIES="cxxflags=-stdlib=libc++ linkflags=-stdlib=libc++"; fi
./b2 "--toolset=${1}" ${PROPERTIES} --with-iostreams --with-regex
popd
popd
