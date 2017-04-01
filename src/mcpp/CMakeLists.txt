add_library(mcpp SHARED
	log.cpp
	log_level.cpp
	null_log.cpp
	stream_log.cpp
)
target_link_libraries(mcpp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_subdirectory(protocol)
add_subdirectory(test)
