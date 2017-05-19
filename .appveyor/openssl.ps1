pushd C:/MCPP2017
wget https://www.npcglib.org/~stathis/downloads/openssl-1.1.0e-vs2017.7z -OutFile openssl.7z
7z x openssl.7z
wget https://raw.githubusercontent.com/Kitware/CMake/master/Modules/FindOpenSSL.cmake -OutFile FindOpenSSL.cmake
cp FindOpenSSL.cmake "C:/Program Files (x86)/CMake/share/cmake-3.8/Modules/FindOpenSSL.cmake"
popd
