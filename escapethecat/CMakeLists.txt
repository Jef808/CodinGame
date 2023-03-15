# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${CMAKE_PROJECT_NAME}_LIB catandmouse.cpp)

add_executable(${CMAKE_PROJECT_NAME}_MAIN main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME}_MAIN PRIVATE ${CMAKE_PROJECT_NAME}_LIB)

find_package(Python3 COMPONENTS Interpreter Development)

add_subdirectory(tests)

add_custom_target(${CMAKE_PROJECT_NAME}_bundled
  ALL
  COMMAND ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -s ${CMAKE_CURRENT_SOURCE_DIR} -d ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_PROJECT_NAME}_bundled
  DEPENDS ${CMAKE_PROJECT_NAME}_MAIN
  BYPRODUCTS "{CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CURRENT_PROJECT_NAME}_bundled.cpp"
  COMMENT "Running ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -d ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_PROJECT_NAME}_bundled -s ${CMAKE_CURRENT_SOURCE_DIR}"
)
