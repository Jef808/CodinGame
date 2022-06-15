cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sok2-local
  types.h
  io.h
  utils.h
  referee.h
  sok2-local.cpp)

add_executable(sok2-binary
  types.h
  io.h
  utils.h
  referee.h
  sok2-binary.cpp)
