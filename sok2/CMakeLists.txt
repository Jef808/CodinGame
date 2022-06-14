cmake_minimum_required(VERSION 3.22)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(sok2
  types.h
  io.h
  utils.h
  fakejudge.h
  sok2.cpp)


add_executable(sok2-local
  types.h
  io.h
  utils.h
  fakejudge.h
  sok2-local.cpp)
