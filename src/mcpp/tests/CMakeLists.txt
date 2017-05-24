add_executable(mcpp_tests
	allocate_unique.cpp
	async.cpp
	checked.cpp
	log.cpp
	main.cpp
	optional.cpp
	polymorphic_ptr.cpp
	stream_log.cpp
)
target_link_libraries(mcpp_tests
	mcpp
	mcpp_test
	Catch
)
add_test(NAME mcpp COMMAND mcpp_tests)
