add_executable(${CMAKE_PROJECT_NAME}_MAIN main.cpp helpers.cpp)
target_link_directories(${CMAKE_PROJECT_NAME}_MAIN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Python3 COMPONENTS Interpreter Development)

set(scripts_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

add_custom_target(${CMAKE_PROJECT_NAME}_bundled
  ALL
  COMMAND ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -s ${CMAKE_CURRENT_SOURCE_DIR} -o ${CMAKE_PROJECT_NAME}_bundled -d ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${CMAKE_PROJECT_NAME}_MAIN
  BYPRODUCTS "{CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}_bundled.cpp"
  COMMENT "Running ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -s ${CMAKE_CURRENT_SOURCE_DIR} -o ${CMAKE_PROJECT_NAME}_bundled -d ${CMAKE_CURRENT_BINARY_DIR}"
)
