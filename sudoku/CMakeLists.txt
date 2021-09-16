cmake_minimum_required( VERSION 3.20 )
project( sudoku )

set( CMAKE_EXPORT_COMPILE_COMMANDS on )
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )
set( CMAKE_BUILD_TYPE release )

add_executable( sudoku sudoku.cpp )
