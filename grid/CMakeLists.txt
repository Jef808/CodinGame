set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED 17 )
set( CMAKE_EXPORT_COMPILE_COMMANDS on )

find_package( SFML COMPONENTS graphics REQUIRED )

# textures used by the viewer
set( resources_dir ${CMAKE_SOURCE_DIR}/resources )

add_library( gridview INTERFACE viewer.h )
target_link_libraries( gridview INTERFACE sfml-graphics sfml-window sfml-system )
#target_include_directories( gridview PUBLIC ${CMAKE_SOURCE_DIR} )

# add_custom_command(
#   TARGET gridview POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy_directory
#           ${resources_dir}
#           ${CMAKE_BINARY_DIR}/resources)
