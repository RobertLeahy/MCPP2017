add_executable(mcpp_protocol_tests
	../../tests/main.cpp
	handshaking.cpp
	incremental_varint_parser.cpp
	int.cpp
	packet_serializer_map.cpp
	string.cpp
	varint.cpp
)
target_link_libraries(mcpp_protocol_tests mcpp mcpp_protocol ${Boost_LIBRARIES})
target_include_directories(mcpp_protocol_tests PRIVATE ${CATCH_INCLUDE_DIR})
add_test(NAME mcpp_protocol COMMAND mcpp_protocol_tests)
