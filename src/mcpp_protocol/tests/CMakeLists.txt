add_executable(mcpp_protocol_tests
	../../mcpp/tests/main.cpp
	handshaking.cpp
	incremental_varint_parser.cpp
	int.cpp
	packet_serializer_map.cpp
	stream_serializer.cpp
	string.cpp
	varint.cpp
)
target_link_libraries(mcpp_protocol_tests
	mcpp
	mcpp_protocol
	Boost::boost
	Boost::iostreams
	Catch
	Expected
	ZLIB::ZLIB
)
add_test(NAME mcpp_protocol COMMAND mcpp_protocol_tests)
