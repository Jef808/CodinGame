find_package( SFML COMPONENTS graphics REQUIRED )

add_library(viewer_data_dir INTERFACE)
target_compile_definitions(viewer_data_dir INTERFACE
  "-DVIEWER_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources\"")
link_libraries(viewer_data_dir)

add_executable(helloworld helloworld.cpp)
add_executable(tilemap tilemap.h tilemap.cpp)
add_executable(viewer tilemap.h viewer.cpp)
target_link_libraries( helloworld PUBLIC
  sfml-graphics
  sfml-window
  sfml-system )
target_link_libraries( tilemap PUBLIC
  sfml-graphics
  sfml-window
  sfml-system )
target_link_libraries( viewer PUBLIC
  sfml-graphics
  sfml-window
  sfml-system )
