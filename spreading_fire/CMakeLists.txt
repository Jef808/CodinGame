set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(spreading_fire_LIB_SOURCES
  enum_utils.h
  point.h
  cell.h
  cell.cpp
  game.h
  game.cpp
  )
add_library(sf_LIB ${spreading_fire_LIB_SOURCES})
target_include_directories(sf_LIB PUBLIC
  ${CMAKE_SOURCE_DIR})

set(spreading_fire_SOURCES
  #enum_utils.h
  #point.h
  pointset.h
  distance.h
  distance.cpp
  #cell.h
  #cell.cpp
  #game.h
  #game.cpp
  agent.h
  agent.cpp)

add_executable(sf
  ${spreading_fire_SOURCES}
  main.cpp)
target_link_libraries(sf sf_LIB)

add_custom_target(sf_main_bundle ALL
  COMMAND /home/jfa/projects/CodinGame/scripts/bundler.py /home/jfa/projects/CodinGame/spreading_fire/
  SOURCES ${spreading_fire_SOURCES} main.cpp)

include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(tests)
