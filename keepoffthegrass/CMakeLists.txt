cmake_minimum_required(VERSION 3.25)
project(keepoffthegrass)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_custom_target(main_keepoffthegrass_bundled
  ALL
  COMMAND ../scripts/bundler.py ${CMAKE_CURRENT_SOURCE_DIR}/sources.txt
  DEPENDS main_keepoffthegrass
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/main_bundled.cpp"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Bundled all the .h and .cpp files into one for submission on the CodinGame website"
)
