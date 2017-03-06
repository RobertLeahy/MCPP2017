add_library(mcpp SHARED
)
target_link_libraries(mcpp ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_subdirectory(test)
