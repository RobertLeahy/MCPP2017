add_library(mcpp_iostreams INTERFACE)
target_include_directories(mcpp_iostreams
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(mcpp_iostreams
	INTERFACE
		mcpp
		Boost::boost
)
add_subdirectory(tests)
