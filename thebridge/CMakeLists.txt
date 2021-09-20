cmake_minimum_required( VERSION 3.20 )
project( The_bridge )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED 17 )
set( CMAKE_BUILD_TYPE debug )

set( SOURCES
  main.cpp
  game.cpp
  search.cpp
  )

add_executable( tb ${SOURCES} )
target_include_directories( tb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

# add_executable( thebridge thebridge.cpp )
# target_include_directories( thebridge PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
