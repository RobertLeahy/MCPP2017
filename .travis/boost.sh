#!/bin/bash
mkdir boost
pushd boost
wget -nc https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.gz
tar -zxf boost_1_64_0.tar.gz
pushd boost_1_64_0
./bootstrap.sh
./b2 --toolset=gcc-6 --with-iostreams --with-regex
popd
popd
