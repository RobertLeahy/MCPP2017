pushd C:/MCPP2017
wget https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.zip -OutFile boost_1_64_0.zip
7z x ./boost_1_64_0.zip
pushd boost_1_64_0
./bootstrap.bat
./b2 toolset=msvc-14.1 address-model=64 --with-iostreams --with-regex --with-system -s ZLIB_BINARY=zlibstatic -s ZLIB_INCLUDE=C:/MCPP2017/zlib-master -s ZLIB_LIBPATH=C:/MCPP2017/zlib-master/build/Release
pushd stage/lib
cp libboost_iostreams-vc141-mt-1_64.lib boost_iostreams-vc141-mt-1_64.lib
cp libboost_iostreams-vc141-mt-gd-1_64.lib boost_iostreams-vc141-mt-gd-1_64.lib
cp libboost_regex-vc141-mt-1_64.lib boost_regex-vc141-mt-1_64.lib
cp libboost_regex-vc141-mt-gd-1_64.lib boost_regex-vc141-mt-gd-1_64.lib
cp libboost_system-vc141-mt-1_64.lib boost_system-vc141-mt-1_64.lib
cp libboost_system-vc141-mt-gd-1_64.lib boost_system-vc141-mt-gd-1_64.lib
popd
popd
popd
