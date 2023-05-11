cmake_minimum_required(VERSION 3.22)
project(
  CodinGame
  HOMEPAGE_URL https://github.com/Jef808/CodinGame/)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(scripts_DIR ${CMAKE_SOURCE_DIR}/scripts)

# Test framework
include(Catch2)
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

# Grid utils
add_subdirectory(grid)

add_library(CG INTERFACE point/point.h grid/grid.h grid/bfs.h)
target_include_directories(CG INTERFACE ${CMAKE_SOURCE_DIR})

project(EscapeTheCat)
add_subdirectory(escapethecat)

project(DontPanic)
add_subdirectory(dontpanic)

project(TowerDereference)
add_subdirectory(tower-dereference)

project(KeepOffTheGrass)
add_subdirectory(kog)


# add_custom_target(main_bundled
#   "./${scripts_DIR}/bundler.py ${scripts_DIR}/sources.txt"
#   DEPENDS
#   BYPRODUCTS ${CMAKE_BINARY_DIR}/main_bundled.cpp
#   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#   COMMENT "Bundled all the .h and .cpp files into one for submission on the CodinGame website"
# )
