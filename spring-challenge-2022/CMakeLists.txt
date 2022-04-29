cmake_minimum_required( VERSION 3.23 )
project( spring-challenge-2022 )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )



######################
# Executable targets #
######################
add_compile_options( -Wall -Wextra -pedantic -std=c++17 )
add_executable( main main.cpp )
target_include_directories( main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

#####################
# CodinGame Bundler #
#####################
set( scripts_DIR /home/jfa/projects/CodinGame/scripts )

add_custom_target(main_bundled
  "${scripts_DIR}/bundler.py" "spring-challenge-2022"
  DEPENDS main
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/main_bundled.cpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Bundle all the .h and .cpp files into one for submission on the CodinGame website"
)

#################
# Dev utilities #
#################
set( spring_challenge_TARGETS
  main
  )

set(GCC_OUTPUT_FORMATTING
  # "-fdiagnostics-color=always"
  # "-fdiagnostics-show-template-tree"
  # "-fdiagnostics-path-format=inline-events"
    # "-fdiagnostics-plain-output"
  )

include(CMakePrintHelpers)
foreach ( target ${spring_challenge_TARGETS} )
  #target_compile_options( ${target} PUBLIC ${GCC_OUTPUT_FORMATTING} )
  cmake_print_properties(
    TARGETS
    ${target}
    PROPERTIES
    COMPILE_OPTIONS
    )
endforeach()
