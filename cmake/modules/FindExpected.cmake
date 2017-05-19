include(ExternalProject)
find_package(Git REQUIRED)
ExternalProject_Add(
	expected
	PREFIX ${CMAKE_BINARY_DIR}/expected
	GIT_REPOSITORY https://github.com/ptal/expected.git
	TIMEOUT 10
	UPDATE_COMMAND ${GIT_EXECUTABLE} pull
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
	LOG_DOWNLOAD ON
)
ExternalProject_Get_Property(expected source_dir)
add_library(Expected INTERFACE)
add_dependencies(Expected expected)
target_include_directories(Expected SYSTEM INTERFACE ${source_dir}/include)
target_link_libraries(Expected INTERFACE Boost::boost)
