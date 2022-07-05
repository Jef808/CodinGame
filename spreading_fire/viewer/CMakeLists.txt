find_package( SFML COMPONENTS graphics REQUIRED )

add_library(viewer_data_dir INTERFACE)
target_compile_definitions(viewer_data_dir INTERFACE
  "-DVIEWER_DATA_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/resources\"")
link_libraries(viewer_data_dir)

add_library(sf_viewer INTERFACE viewer.h tilemap.h)

target_link_libraries( sf_viewer INTERFACE
  sfml-graphics
  sfml-window
  sfml-system
  viewer_data_dir
  sf_LIB)


add_executable(helloworld helloworld.cpp)

target_link_libraries( helloworld PUBLIC
  sfml-graphics
  sfml-window
  sfml-system )



# add_executable(viewer_example viewer_example.cpp)

# target_link_libraries( viewer_example PUBLIC
#   sfml-graphics
#   sfml-window
#   sfml-system
#   sf_LIB)
