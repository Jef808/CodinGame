set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests)

add_executable(grid_tests test_grid.cpp helpers.cpp)

# target_compile_options(grid_tests PRIVATE "-fsanitize=address")
# target_link_options(grid_tests PRIVATE "-fsanitize=address")

target_link_libraries(grid_tests PRIVATE CG Catch2::Catch2WithMain)

catch_discover_tests(grid_tests)
