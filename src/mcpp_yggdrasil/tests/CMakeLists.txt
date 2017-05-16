add_executable(mcpp_yggdrasil_tests
	../../mcpp/tests/main.cpp
	http.cpp
	json.cpp
)
target_link_libraries(mcpp_yggdrasil_tests
	mcpp
	mcpp_yggdrasil
	Asio
	BeastExtras
	Boost::boost
	Catch
)
add_test(NAME mcpp_yggdrasil COMMAND mcpp_yggdrasil_tests)
