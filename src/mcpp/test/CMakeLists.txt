add_executable(tests
	main.cpp
)
target_link_libraries(tests mcpp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(tests PRIVATE ${CATCH_INCLUDE_DIR})
add_test(NAME tests COMMAND tests)
