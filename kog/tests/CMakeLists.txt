set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests)

add_executable(agent_tests
  test_battlefronts.cpp
  ../voronoi_helper.cpp
  ../game.cpp)
target_link_libraries(agent_tests PRIVATE CG Catch2::Catch2WithMain)

catch_discover_tests(agent_tests)
