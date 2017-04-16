add_executable(mcpp_iostreams_tests
	../../tests/main.cpp
	limiting_source.cpp
	offset.cpp
	proxy_sink.cpp
	proxy_source.cpp
	traits.cpp
)
target_link_libraries(mcpp_iostreams_tests mcpp ${Boost_LIBRARIES})
target_include_directories(mcpp_iostreams_tests PRIVATE ${CATCH_INCLUDE_DIR})
add_test(NAME mcpp_iostreams COMMAND mcpp_iostreams_tests)
