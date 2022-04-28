project( spring-challenge-2022 )

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

add_executable( main main.cpp )


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


include(CMakePrintHelpers)
foreach ( target ${escapethecat_TARGETS} )
  target_compile_options( ${target} ${GCC_OUTPUT_FORMATTING} )
  cmake_print_properties(
    TARGETS
    ${target}
    PROPERTIES
    COMPILE_OPTIONS
    )
endforeach()
