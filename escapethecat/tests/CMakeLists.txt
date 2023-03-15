set(EXECUTABLE_OUTPUT_PATH
  ${CMAKE_BINARY_DIR}/tests/${CMAKE_PROJECT_NAME})

add_executable(${CMAKE_PROJECT_NAME}_tests test.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME}_tests PRIVATE Catch2::Catch2WithMain)

catch_discover_tests(${CMAKE_PROJECT_NAME}_tests)
