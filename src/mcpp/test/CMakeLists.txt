add_executable(mcpp_test
	log.cpp
	main.cpp
	stream_log.cpp
)
target_link_libraries(mcpp_test mcpp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(mcpp_test PRIVATE ${CATCH_INCLUDE_DIR})
add_test(NAME mcpp COMMAND mcpp_test)
