cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(rapid_json_builder CXX)
include(ExternalProject)
find_package(Git REQUIRED)
ExternalProject_Add(
	rapid_json
	PREFIX ${CMAKE_BINARY_DIR}/rapid_json
	GIT_REPOSITORY https://github.com/miloyip/rapidjson.git
	TIMEOUT 10
	UPDATE_COMMAND ${GIT_EXECUTABLE} pull
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
	LOG_DOWNLOAD ON
)
ExternalProject_Get_Property(rapid_json source_dir)
add_library(RapidJSON INTERFACE)
add_dependencies(RapidJSON rapid_json)
target_include_directories(RapidJSON SYSTEM INTERFACE ${source_dir}/include)
