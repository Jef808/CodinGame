
add_library(test_data_dir INTERFACE)
target_compile_definitions(test_data_dir INTERFACE
  "-DTEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\"")
link_libraries(test_data_dir)


add_executable(test_game
  test_game.cc
  ${CMAKE_SOURCE_DIR}/agent.h
  ${CMAKE_SOURCE_DIR}/agent.cpp
  ${CMAKE_SOURCE_DIR}/distance.h
  ${CMAKE_SOURCE_DIR}/distance.cpp
  )
target_link_libraries(test_game PUBLIC sf_LIB)
