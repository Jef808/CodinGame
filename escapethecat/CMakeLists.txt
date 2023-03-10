link_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(EscapeTheCat entities.cpp)

add_executable(escape_the_cat main.cpp)
target_link_libraries(escape_the_cat PRIVATE EscapeTheCat)

find_package(Python3 COMPONENTS Interpreter Development)

add_custom_target(escape_the_cat_bundled
  ALL
  COMMAND "${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py ${CMAKE_CURRENT_SOURCE_DIR}/sources.txt"
  DEPENDS escape_the_cat
  BYPRODUCTS "{CMAKE_CURRENT_BINARY_DIR}/escape_the_cat_bundled.cpp"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Bundled all the .h and .cpp files into one for submission on the CodinGame website"
)
