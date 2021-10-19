cmake_minimum_required( VERSION 3.19 )
project( DontPanic )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_BUILD_TYPE debug )

set( SOURCES
  main.cpp
  dp.cpp
  agent.cpp
  )

find_package(fmt)

add_executable( main ${SOURCES} )
target_link_libraries( main fmt::fmt )
target_include_directories( main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
