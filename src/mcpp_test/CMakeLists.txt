add_library(mcpp_test SHARED
	allocator.cpp
	object.cpp
)
target_include_directories(mcpp_test
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(mcpp_test
	mcpp
)
