add_library(mcpp_protocol SHARED
	error.cpp
	exception.cpp
)
target_link_libraries(mcpp_protocol mcpp ${Boost_LIBRARIES})
