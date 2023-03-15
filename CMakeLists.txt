cmake_minimum_required(VERSION 3.22)
project(
  CodinGame
  HOMEPAGE_URL https://github.com/Jef808/CodinGame/)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(scripts_DIR ${CMAKE_SOURCE_DIR}/scripts)

include(Catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)

project(EscapeTheCat)
add_subdirectory(escapethecat)

# add_subdirectory(fall2022)
#add_subdirectory(platinumRift)
#add_subdirectory(deathfirstsearch1)



# add_custom_target(main_bundled
#   "./${scripts_DIR}/bundler.py ${scripts_DIR}/sources.txt"
#   DEPENDS
#   BYPRODUCTS ${CMAKE_BINARY_DIR}/main_bundled.cpp
#   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#   COMMENT "Bundled all the .h and .cpp files into one for submission on the CodinGame website"
# )
