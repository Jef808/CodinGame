include(imgui-glfw-glad-glm)

add_executable(viewer)

target_sources(viewer
  PUBLIC
  viewer.cpp
  )

target_link_libraries(viewer
  imgui
  glm
  )

target_compile_features(viewer PRIVATE cxx_std_17)

list(APPEND escape_cat_TARGETS viewer)
