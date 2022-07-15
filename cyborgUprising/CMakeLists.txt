cmake_minimum_required( VERSION 3.23 )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

######################
# Sources            #
######################
set(cyborgUprising_SOURCES
  game.h
  game.cpp
  search.h
  search.cpp
  main.cpp)

set(typelist_DIR
  /home/jfa/projects/typelist)

######################
# Executable targets #
######################
add_compile_options( -Wall -Wextra -pedantic -std=c++17 )
add_executable( main ${cyborgUprising_SOURCES} )
target_include_directories( main PUBLIC
  typelist_DIR)
#target_include_directories( main PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

#####################
# CodinGame Bundler #
#####################
set( scripts_DIR /home/jfa/projects/CodinGame/scripts )

add_custom_target(main_bundled ALL
  "${scripts_DIR}/bundler.py" "cyborgUprising"
  DEPENDS main
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/main_bundled.cpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Bundle all the .h and .cpp files into one file for submission on the CodinGame website"
)

#################
# Print utilities #
#################
set( spring_challenge_TARGETS
  main
  )

include(CMakePrintHelpers)
foreach ( target ${spring_challenge_TARGETS} )
  cmake_print_properties(
    TARGETS
    ${target}
    PROPERTIES
    COMPILE_OPTIONS
    )
endforeach()
