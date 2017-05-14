add_executable(mcpp_iostreams_tests
	../../mcpp/tests/main.cpp
	concatenating_source.cpp
	limiting_source.cpp
	offset.cpp
	proxy_sink.cpp
	proxy_source.cpp
	traits.cpp
)
target_link_libraries(mcpp_iostreams_tests
	mcpp
	mcpp_iostreams
	Boost::boost
	Catch
)
add_test(NAME mcpp_iostreams COMMAND mcpp_iostreams_tests)
