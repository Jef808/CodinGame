project( breakthrough )
cmake_minimum_required( VERSION 3.21 )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_BUILD_TYPE debug )

add_executable( bt main.cpp breakthrough.cpp agent.cpp eval.cpp )

add_executable( debug debug.cpp breakthrough.cpp agent.cpp eval.cpp )

add_custom_command(
  TARGET bt POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/data
          ${CMAKE_BINARY_DIR}/data
  )

add_custom_command(
  TARGET bt POST_BUILD
  COMMAND ../scripts/bundler
  BYPRODUCT btbundled.cpp
)
