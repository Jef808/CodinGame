set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED 17 )
set( CMAKE_EXPORT_COMPILE_COMMANDS on )

# textures used by the viewer
set( resources_dir ${CMAKE_SOURCE_DIR}/resources )

add_subdirectory( tests )

# target_link_libraries( grid INTERFACE sfml-graphics sfml-window sfml-system )
# target_include_directories( gridview PUBLIC ${CMAKE_SOURCE_DIR} )
