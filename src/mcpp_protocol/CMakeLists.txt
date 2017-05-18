add_library(mcpp_protocol SHARED
	direction.cpp
	error.cpp
	exception.cpp
	packet.cpp
	packet_id.cpp
	state.cpp
)
include(CheckIncludeFileCXX)
set(CMAKE_REQUIRED_INCLUDES ${Boost_INCLUDE_DIRS})
check_include_file_cxx(boost/endian/conversion.hpp HAS_BOOST_ENDIAN_CONVERSION)
unset(CMAKE_REQUIRED_INCLUDES)
if(HAS_BOOST_ENDIAN_CONVERSION)
	target_compile_definitions(mcpp_protocol PUBLIC -DMCPP_HAS_BOOST_ENDIAN_CONVERSION)
endif()
target_include_directories(mcpp_protocol
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(mcpp_protocol
	mcpp
	mcpp_iostreams
	Boost::boost
	Boost::iostreams
	Expected
	ZLIB::ZLIB
)
add_subdirectory(tests)
