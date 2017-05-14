add_library(mcpp_yggdrasil SHARED
	agent.cpp
	authenticate.cpp
	profile.cpp
	user.cpp
)
target_include_directories(mcpp_yggdrasil
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(mcpp_yggdrasil
	mcpp
)
add_subdirectory(tests)
