add_executable(mcpp_yggdrasil_tests
	../../mcpp/tests/main.cpp
	json.cpp
)
target_link_libraries(mcpp_yggdrasil_tests
	mcpp_yggdrasil
	Catch
)
add_test(NAME mcpp_yggdrasil COMMAND mcpp_yggdrasil_tests)
