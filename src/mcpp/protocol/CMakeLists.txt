add_library(mcpp_protocol SHARED
	direction.cpp
	error.cpp
	exception.cpp
	packet.cpp
	packet_id.cpp
	state.cpp
)
target_link_libraries(mcpp_protocol mcpp ${Boost_LIBRARIES})
add_subdirectory(tests)
