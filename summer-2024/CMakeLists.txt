cmake_minimum_required( VERSION 3.29 )
project( summer_2024 )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_BUILD_TYPE debug )

set( SOURCES
  olymbits.cpp
  agent.cpp
  main.cpp
  )

add_executable( main main.cpp ${SOURCES} )
target_include_directories( main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

set( cg_dir ~/projects/codingame )
message( STATUS "cg_dir: " ${cg_dir} )

add_custom_command(
  TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/data
          ${CMAKE_BINARY_DIR}/data
  )

add_custom_command(
  TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/view/resources
          ${CMAKE_BINARY_DIR}/resources
  )

add_custom_command(
  TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${cg_dir}/grid/resources
          ${CMAKE_BINARY_DIR}/resources
  )
