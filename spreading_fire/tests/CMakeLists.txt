
add_library(test_data_dir INTERFACE)
target_compile_definitions(test_data_dir INTERFACE
  "-DTEST_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/data\"")
link_libraries(test_data_dir)

set(SF_TESTS
  test_play
  test_game
  test_search
  test_agent_randommove
  test_agent_distancemap
  )

foreach(test ${SF_TESTS})
  add_executable(${test}
    ${test}.cc
    )
target_link_libraries(${test} PUBLIC sf_LIB sf_viewer_LIB)
target_link_libraries(${test} PUBLIC sf_LIB sf_search_LIB sf_viewer_LIB)
endforeach()

# add_executable(debug_agent_distancemap
#   debug_agent_distancemap.cc
#   )
# target_link_libraries(debug_agent_distancemap
#   sf_LIB
#   sf_viewer)
