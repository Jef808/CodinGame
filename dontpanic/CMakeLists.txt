#set( CMAKE_CXX_STANDARD 20 )
#set( CMAKE_CXX_STANDARD_REQUIRED 20 )

set( main_SOURCES
  main.cpp
  dp.cpp
  agent.cpp
  )
set( libs  )

# set( view_dir ${CMAKE_SOURCE_DIR}/view )

find_package(fmt)
list( APPEND libs fmt )

add_subdirectory(view)
list( APPEND libs dpview )

message( STATUS "Libraries:" ${libs} )

add_executable(${CMAKE_PROJECT_NAME}_MAIN
  main.cpp
  dp.cpp
  agent.cpp
  mgr.cpp)
target_link_libraries( ${CMAKE_PROJECT_NAME}_MAIN ${libs} )
target_include_directories( ${CMAKE_PROJECT_NAME}_MAIN PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( ${CMAKE_PROJECT_NAME}_human_play mgr_main.cpp mgr.cpp dp.cpp )
target_link_libraries( ${CMAKE_PROJECT_NAME}_human_play ${libs} )
target_include_directories( ${CMAKE_PROJECT_NAME}_human_play PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

add_executable( ${CMAKE_PROJECT_NAME}_viewer viewer.cpp dp.cpp )
target_link_libraries( ${CMAKE_PROJECT_NAME}_viewer ${libs} )
target_include_directories( ${CMAKE_PROJECT_NAME}_viewer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

find_package(Python3 COMPONENTS Interpreter Development)
add_custom_target(${CMAKE_PROJECT_NAME}_bundled
  ALL
  COMMAND ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -s ${CMAKE_CURRENT_SOURCE_DIR} -d ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_PROJECT_NAME}_bundled
  DEPENDS ${CMAKE_PROJECT_NAME}_MAIN
  BYPRODUCTS "{CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CURRENT_PROJECT_NAME}_bundled.cpp"
  COMMENT "Running ${Python3_EXECUTABLE} ${scripts_DIR}/bundler.py -d ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_PROJECT_NAME}_bundled -s ${CMAKE_CURRENT_SOURCE_DIR}"
)
