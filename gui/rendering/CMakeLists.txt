target_sources(
  gui
  PRIVATE Camera.cpp
          ObjectLoader.cpp
          ShaderProgram.cpp
          Utils.cpp
          WireframeObject.cpp)

target_include_directories(gui PUBLIC .)

if(BUILD_TESTING)
  target_sources(gui_tests PUBLIC)
endif()
