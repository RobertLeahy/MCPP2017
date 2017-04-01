cmake_minimum_required(VERSION 2.8.8)
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
set(EXPECTED_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "Path to include folder for Boost.Expected")
