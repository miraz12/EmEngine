add_library(exts INTERFACE null.cpp)

if(NOT DEFINED EMSCRIPTEN)

  find_package(OpenGL REQUIRED)

  # set(GLFW_USE_WAYLAND ON) set(GLFW_USE_X11 OFF)
  set(GLFW_BUILD_DOCS
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES
      OFF
      CACHE BOOL "" FORCE)
  add_subdirectory(glfw)

endif()

# GLFW
target_include_directories(exts SYSTEM INTERFACE glfw/include)

# GLAD
add_library(glad glad/src/glad.c glad/include/glad/glad.h)
target_include_directories(glad PUBLIC glad/include)

# Bullet3
option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_BULLET3_DEMOS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_PYBULLET OFF)
option(BUILD_UNIT_TESTS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)

add_subdirectory(bullet3)
target_include_directories(exts SYSTEM INTERFACE bullet3/src)

# GLM
add_subdirectory(glm)
target_include_directories(exts SYSTEM INTERFACE glm/include)

# YAML-CPP
add_subdirectory(yaml-cpp)
target_include_directories(exts SYSTEM INTERFACE yaml-cpp/include)

# gtest
add_subdirectory(googletest)
target_include_directories(exts SYSTEM INTERFACE googletest/src)

# IMGUI
add_library(
  imgui
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_internal.h
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imstb_rectpack.h
  imgui/imstb_textedit.h
  imgui/imstb_truetype.h
  imgui/backends/imgui_impl_glfw.h
  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_opengl3.h
  imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC imgui glfw/include)

# IMGUIZMO add_library( imguizmo ImGuizmo/GraphEditor.cpp ImGuizmo/GraphEditor.h
# ImGuizmo/ImCurveEdit.cpp ImGuizmo/ImCurveEdit.h ImGuizmo/ImGradient.cpp
# ImGuizmo/ImGradient.h ImGuizmo/ImGuizmo.cpp ImGuizmo/ImGuizmo.h
# ImGuizmo/ImSequencer.cpp ImGuizmo/ImSequencer.h ImGuizmo/ImZoomSlider.h)
# target_include_directories(imguizmo PUBLIC ImGuizmo imgui)

# Header only
target_include_directories(exts INTERFACE header_only)

target_link_libraries(
  exts
  INTERFACE glfw
            ${OPENGL_LIBRARIES}
            glm
            imgui
            BulletDynamics
            BulletCollision
            LinearMath
            yaml-cpp)
# imguizmo)

if(NOT DEFINED EMSCRIPTEN)
  target_link_libraries(exts INTERFACE glad)
endif()
