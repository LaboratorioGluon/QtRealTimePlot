# Plain test executables (no extra framework). Run with `ctest` from the build directory.
# None of them create a window or need an OpenGL context.

foreach(test_name test_sample_buffer test_plot_view test_decimation test_plot_axis)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE realtimeplot)
    # Tests may also exercise internal helpers.
    target_include_directories(${test_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
    add_test(NAME ${test_name} COMMAND ${test_name})
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 120)
endforeach()
