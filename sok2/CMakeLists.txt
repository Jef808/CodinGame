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

add_executable(sok2-main
  types.h
  io.h
  agent.h
  utils.h
  sok2-main.cpp)

add_custom_target(sok2-main-bundle ALL
  COMMAND /home/jfa/projects/CodinGame/scripts/bundler.py /home/jfa/projects/CodinGame/sok2/
  SOURCES types.h io.h agent.h utils.h sok2-main.cpp)
