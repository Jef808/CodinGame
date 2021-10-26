cmake_minimum_required( VERSION 3.19 )
project( The_bridge )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_BUILD_TYPE debug )

set( SOURCES
  tb.cpp
  agent.cpp
  )

add_executable( main main.cpp ${SOURCES} )
target_include_directories( main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

add_custom_command(
  TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/data
          ${CMAKE_BINARY_DIR}/data
  )
