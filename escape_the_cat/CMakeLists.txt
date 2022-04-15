project(mouse-cat-escape)
cmake_minimum_required(VERSION 3.22)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

set( escape_cat_SOURCES
  escape_cat.cpp
  agent.cpp
  )

set(GCC_OUTPUT_FORMATTING
  "SHELL:-fdiagnostics-color=always"
  "SHELL:-fdiagnostics-show-template-tree"
  "SHELL:-fdiagnostics-path-format=inline-events" )

add_executable( escape_cat
  main.cpp
  ${escape_cat_SOURCES} )
target_include_directories(escape_cat PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR})

# add_custom_command(

#   OUTPUT main_bundled
#   MAIN_DEPENDENCY escape_cat
#   BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/main_bundled.cpp

#   ) j

set( scripts_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts )
add_custom_target(main_bundled
  "./${scripts_DIR}/bundler.py ${scripts_DIR}/sources.txt"
  DEPENDS escape_cat
  BYPRODUCTS ${CMAKE_BINARY_DIR}/main_bundled.cpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Bundled all the .h and .cpp files into one for submission on the CodinGame website"
)
