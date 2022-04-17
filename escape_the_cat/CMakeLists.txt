####################
# Generalities     #
####################
cmake_policy( SET CMP0048 NEW )

project( escape-the-cat VERSION 1.0 )
cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../cmake)

###############
# Executables #
###############
set( mouse-cat-escape_TARGETS
  main_escape_cat
  )

foreach ( target ${mouse-cat-escape_TARGETS} )
  add_executable( ${target} )
endforeach()

######################################
# main_escape_cat build instructions #
######################################
target_sources( main_escape_cat
  PUBLIC
  main.cpp
  escape_cat.cpp
  agent.cpp
  )
target_include_directories(main_escape_cat
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  )


#################
# Dev utilities #
#################
set( scripts_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts )

add_custom_target(main_bundled
  "./${scripts_DIR}/bundler.py ${scripts_DIR}/sources.txt"
  DEPENDS main_escape_cat
  BYPRODUCTS ${CMAKE_BINARY_DIR}/main_bundled.cpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Bundled all the .h and .cpp files into one for submission on the CodinGame website"
)


###################
# Sub directories #
###################
add_subdirectory(imgui)



set(GCC_OUTPUT_FORMATTING
  "SHELL:-fdiagnostics-color=always"
  "SHELL:-fdiagnostics-show-template-tree"
  "SHELL:-fdiagnostics-path-format=inline-events" )


include(CMakePrintHelpers)
foreach ( target ${escape_cat_TARGETS} )
  target_compile_options( ${target} ${GCC_OUTPUT_FORMATTING} )

  cmake_print_properties(
    TARGETS
    ${target}
    PROPERTIES
    COMPILE_OPTIONS
    )
endforeach()
