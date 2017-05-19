add_executable(mcpp_yggdrasil_tests
	../../mcpp/tests/main.cpp
	http.cpp
	json.cpp
)
if(YGGDRASIL_USERNAME)
	set(MCPP_YGGDRASIL_USERNAME ${YGGDRASIL_USERNAME})
endif()
if(YGGDRASIL_PASSWORD)
	set(MCPP_YGGDRASIL_PASSWORD ${YGGDRASIL_PASSWORD})
endif()
configure_file(config.hpp.in config.hpp ESCAPE_QUOTES)
target_include_directories(mcpp_yggdrasil_tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(mcpp_yggdrasil_tests
	mcpp
	mcpp_yggdrasil
	Asio
	BeastExtras
	Boost::boost
	Catch
	OpenSSL::Crypto
	OpenSSL::SSL
)
add_test(NAME mcpp_yggdrasil COMMAND mcpp_yggdrasil_tests)
