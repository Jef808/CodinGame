project( breakthrough )
cmake_minimum_required( VERSION 3.21 )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_BUILD_TYPE debug )
set( CMAKE_VERBOSE_MAKEFILE on )

set( data_DIR ${CMAKE_SOURCE_DIR}/data )
set( scripts_DIR ${CMAKE_SOURCE_DIR}/scripts )

add_executable( bt main.cpp breakthrough.cpp agent.cpp eval.cpp )

add_executable( benchmark benchmark.cpp breakthrough.cpp agent.cpp eval.cpp )

add_executable( debug debug.cpp breakthrough.cpp agent.cpp eval.cpp )

add_custom_command(
  TARGET bt POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${data_DIR} data
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${scripts_DIR} scripts
)
