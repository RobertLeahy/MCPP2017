include(CheckIncludeFileCXX)
check_include_file_cxx(optional HAS_OPTIONAL)
check_include_file_cxx(experimental/optional HAS_EXPERIMENTAL_OPTIONAL)
add_library(Optional INTERFACE)
if(HAS_OPTIONAL)
	target_compile_definitions(Optional INTERFACE -DMCPP_HAS_OPTIONAL)
endif()
if(HAS_EXPERIMENTAL_OPTIONAL)
	target_compile_definitions(Optional INTERFACE -DMCPP_HAS_EXPERIMENTAL_OPTIONAL)
endif()
if(NOT (HAS_OPTIONAL OR HAS_EXPERIMENTAL_OPTIONAL))
	include(ExternalProject)
	find_package(Git REQUIRED)
	ExternalProject_Add(
		optional
		PREFIX ${CMAKE_BINARY_DIR}/optional
		GIT_REPOSITORY https://github.com/RobertLeahy/Optional-1.git
		TIMEOUT 10
		UPDATE_COMMAND ${GIT_EXECUTABLE} pull
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
		LOG_DOWNLOAD ON
	)
	ExternalProject_Get_Property(optional source_dir)
	add_dependencies(Optional optional)
	target_include_directories(Optional SYSTEM INTERFACE ${source_dir})
endif()
