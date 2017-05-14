add_library(mcpp SHARED
	log.cpp
	log_level.cpp
	null_log.cpp
	stream_log.cpp
)
target_include_directories(mcpp
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(mcpp
	Threads::Threads
)
add_subdirectory(tests)
