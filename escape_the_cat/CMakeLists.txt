project(mouse-cat-escape)
cmake_minimum_required(VERSION 3.22)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

set( escape_cat_SOURCES
  escape_cat.cpp
  agent.cpp
  )

add_executable( escape_cat
  main.cpp
  ${escape_cat_SOURCES} )
target_include_directories(escape_cat PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})
