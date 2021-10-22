cmake_minimum_required( VERSION 3.21 )
project( DontPanic )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_BUILD_TYPE debug )
set( CMAKE_VERBOSE_MAKEFILE on )

set( with_view on )
set( with_fmt on )

set( main_SOURCES
  main.cpp
  dp.cpp
  agent.cpp
  )
set( libs  )

#set( data-dir ${CMAKE_SOURCE_DIR}/data )
set( view_dir ${CMAKE_SOURCE_DIR}/view )

if ( with_fmt )
  find_package(fmt)
  list( APPEND libs fmt )
endif()
if ( with_view )
  add_subdirectory( ${view_dir} )
endif()

add_executable( main ${main_SOURCES} )
target_link_libraries( main ${libs} )
target_include_directories( main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

add_custom_command(
  TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/data
          ${CMAKE_BINARY_DIR}/data
  )
