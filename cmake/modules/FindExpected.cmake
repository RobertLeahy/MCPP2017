cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(expected_builder CXX)
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
target_include_directories(Expected INTERFACE ${source_dir}/include)
target_link_libraries(Expected INTERFACE Boost::boost)
