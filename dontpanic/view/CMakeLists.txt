# sfml viewer for don't panic

find_package( SFML COMPONENTS graphics REQUIRED )

# textures used by the viewer
set( resources_dir ${CMAKE_CURRENT_SOURCE_DIR}/resources )

add_library( dpview dpview.cpp )
target_link_libraries( dpview PUBLIC sfml-graphics sfml-window sfml-system )
target_include_directories( dpview PUBLIC ${CMAKE_SOURCE_DIR} )

add_custom_command(
  TARGET dpview POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${resources_dir}
          ${CMAKE_BINARY_DIR}/resources)
