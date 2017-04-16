add_executable(mcpp_tests
	allocate_unique.cpp
	checked.cpp
	log.cpp
	main.cpp
	optional.cpp
	stream_log.cpp
)
target_link_libraries(mcpp_tests mcpp mcpp_test ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(mcpp_tests PRIVATE ${CATCH_INCLUDE_DIR})
add_test(NAME mcpp COMMAND mcpp_tests)
