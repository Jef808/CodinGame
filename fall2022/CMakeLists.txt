cmake_minimum_required(VERSION 3.25)
project(fall2022)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

add_executable(fall2022-main fall2022_main.cpp)

add_custom_target(fall2022-main-bundled
  ALL
  COMMAND ../scripts/bundler.py ${CMAKE_CURRENT_SOURCE_DIR}/sources.txt
  DEPENDS fall2022-main
  BYPRODUCTS "${CMAKE_BINARY_DIR}/main_bundled.cpp"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Bundled all the .h and .cpp files into one for submission on the CodinGame website"
)
