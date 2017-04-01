add_library(mcpp_protocol SHARED
	error.cpp
	error_code.cpp
)
target_link_libraries(mcpp_protocol mcpp ${Boost_LIBRARIES})
