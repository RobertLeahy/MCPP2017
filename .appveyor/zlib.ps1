pushd C:/MCPP2017
wget https://github.com/madler/zlib/archive/master.zip -OutFile zlib.zip
7z x ./zlib.zip
pushd zlib-master
mkdir build
pushd build
cmake .. -G "Visual Studio 15 2017 Win64"
cmake --build . --config Release
cp zconf.h ..
popd
popd
popd
