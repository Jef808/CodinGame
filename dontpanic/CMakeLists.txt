cmake_minimum_required( VERSION 3.21 )
project( DontPanic )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_BUILD_TYPE release )
set( CMAKE_VERBOSE_MAKEFILE on )

set( with_view on )
set( with_fmt on )

set( main_SOURCES
  main.cpp
  dp.cpp
  agent.cpp
  )
set( libs  )

set( view_dir ${CMAKE_SOURCE_DIR}/view )

if ( with_fmt )
  find_package(fmt)
  list( APPEND libs fmt )
endif()
if ( with_view )
  add_subdirectory( ${view_dir} )
  list( APPEND libs dpview )
endif()

message( STATUS "Libraries:" ${libs} )

add_executable( main main.cpp dp.cpp agent.cpp mgr.cpp)
target_link_libraries( main ${libs} )
target_include_directories( main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( human_play mgr_main.cpp mgr.cpp dp.cpp )
target_link_libraries( human_play ${libs} )
target_include_directories( human_play PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( viewer viewer.cpp dp.cpp )
target_link_libraries( viewer ${libs} )
target_include_directories( viewer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( cg_bundle bundled.cpp )
target_link_libraries( cg_bundle fmt::fmt )
target_include_directories( cg_bundle PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

add_custom_command(
  TARGET main POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/data
          ${CMAKE_BINARY_DIR}/data
  )
