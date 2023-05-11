#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD_REQUIRED 20)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${CMAKE_PROJECT_NAME}_MAIN
  main.cpp
  game.cpp
  agent.cpp
)
target_link_libraries(${CMAKE_PROJECT_NAME}_MAIN PRIVATE CG)

add_subdirectory(tests)

find_package(Python3 COMPONENTS Interpreter Development)
add_custom_target(${CMAKE_PROJECT_NAME}_bundled
  ALL
  COMMAND ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -s ${CMAKE_CURRENT_SOURCE_DIR} -d ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_PROJECT_NAME}_bundled
  DEPENDS ${CMAKE_PROJECT_NAME}_MAIN
  BYPRODUCTS "{CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CURRENT_PROJECT_NAME}_bundled.cpp"
  COMMENT "Running ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -d ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_PROJECT_NAME}_bundled -s ${CMAKE_CURRENT_SOURCE_DIR}"
)
