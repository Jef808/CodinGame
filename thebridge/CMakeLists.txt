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

add_subdirectory( ./../grid grid )
set( view_dir ./view )

set( cg_dir ~/code/projects/codinGame )
message( STATUS "cg_dir: " ${cg_dir} )

add_executable( road tb.cpp ${view_dir}/road.cpp )
target_include_directories( road PUBLIC ${cg_dir} ${CMAKE_SOURCE_DIR} )
target_link_libraries( road PUBLIC gridview )

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
