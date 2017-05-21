param([string]$boost_minor)
pushd C:/MCPP2017
if ($boost_minor -eq "64") {
	$boost_url = "https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.zip"
} else {
	$boost_url = "https://sourceforge.net/projects/boost/files/boost/1.$($boost_minor).0/boost_1_$($boost_minor)_0.zip"
}
$boost_dir = "boost_1_$($boost_minor)_0"
$boost_zip = "$($boost_dir).zip"
wget $boost_url -OutFile $boost_zip -UserAgent [Microsoft.PowerShell.Commands.PSUserAgent]::FireFox
7z x $boost_zip
pushd $boost_dir
./bootstrap.bat
./b2 toolset=msvc-14.1 address-model=64 --with-iostreams --with-regex --with-system -s ZLIB_BINARY=zlibstatic -s ZLIB_INCLUDE=C:/MCPP2017/zlib-master -s ZLIB_LIBPATH=C:/MCPP2017/zlib-master/build/Release
pushd stage/lib
cp libboost_iostreams-vc141-mt-1_$($boost_minor).lib boost_iostreams-vc141-mt-1_$($boost_minor).lib
cp libboost_iostreams-vc141-mt-gd-1_$($boost_minor).lib boost_iostreams-vc141-mt-gd-1_$($boost_minor).lib
cp libboost_regex-vc141-mt-1_$($boost_minor).lib boost_regex-vc141-mt-1_$($boost_minor).lib
cp libboost_regex-vc141-mt-gd-1_$($boost_minor).lib boost_regex-vc141-mt-gd-1_$($boost_minor).lib
cp libboost_system-vc141-mt-1_$($boost_minor).lib boost_system-vc141-mt-1_$($boost_minor).lib
cp libboost_system-vc141-mt-gd-1_$($boost_minor).lib boost_system-vc141-mt-gd-1_$($boost_minor).lib
popd
popd
popd
