set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED 20 )

# textures used by the viewer
set( resources_dir ${CMAKE_SOURCE_DIR}/resources )

add_subdirectory( tests )

# target_link_libraries( grid INTERFACE sfml-graphics sfml-window sfml-system )
# target_include_directories( gridview PUBLIC ${CMAKE_SOURCE_DIR} )
