cmake_minimum_required(VERSION 3.23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(sf_LIB_SOURCES
  cell.h
  cell.cpp
  game.h
  game.cpp
  )
set(sf_search_LIB_SOURCES
  agent.h
  agent.cpp
  search.h
  search.cpp
  )

add_library(sf_LIB ${sf_LIB_SOURCES})
target_include_directories(sf_LIB PUBLIC
  ${CMAKE_SOURCE_DIR})

add_library(sf_search_LIB ${sf_search_LIB_SOURCES})
target_link_libraries(sf_search_LIB sf_LIB)

add_executable(sf
  main.cpp)
target_link_libraries(sf sf_LIB sf_search_LIB)

add_custom_target(sf_main_bundle ALL
  COMMAND /home/jfa/projects/CodinGame/scripts/bundler.py /home/jfa/projects/CodinGame/spreading_fire/
  SOURCES ${sf_SOURCES} main.cpp)

include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(viewer)

add_subdirectory(tests)
