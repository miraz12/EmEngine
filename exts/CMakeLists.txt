add_library(exts INTERFACE null.cpp)

if(LINUX)
  add_compile_options(-Wno-everything)
elseif(WIN32)
  add_compile_options(/W0 /MT;)
endif()

if(NOT DEFINED EMSCRIPTEN)

  find_package(OpenGL REQUIRED)

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
  # GLFW
  target_include_directories(exts SYSTEM INTERFACE glfw/include)
  set_target_properties(
    glfw
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib")

endif()

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

# IMGUIZMO
add_library(
  imguizmo
  ImGuizmo/GraphEditor.cpp
  ImGuizmo/GraphEditor.h
  ImGuizmo/ImCurveEdit.cpp
  ImGuizmo/ImCurveEdit.h
  ImGuizmo/ImGradient.cpp
  ImGuizmo/ImGradient.h
  ImGuizmo/ImGuizmo.cpp
  ImGuizmo/ImGuizmo.h
  ImGuizmo/ImSequencer.cpp
  ImGuizmo/ImSequencer.h
  ImGuizmo/ImZoomSlider.h)
target_include_directories(imguizmo PUBLIC ImGuizmo imgui)
set_target_properties(
  imguizmo
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib")

# FreeType
add_subdirectory(freetype)
set_target_properties(
  freetype
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib")

add_library(Freetype::Freetype ALIAS freetype)

# RmlUi
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "" FORCE)
set(RMLUI_BACKEND GLFW_GL3 CACHE BOOL "" FORCE)
set(RMLUI_SHELL ON CACHE BOOL "" FORCE)

add_subdirectory(RmlUi)
set_target_properties(
  rmlui_core
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib")
set_target_properties(
  rmlui_debugger
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib")

# Configure RmlUi backend to use GLAD and disable MSAA
target_compile_definitions(
  rmlui_backend_GLFW_GL3
    INTERFACE "RMLUI_GL3_CUSTOM_LOADER=<glad/glad.h>"
              "RMLUI_NUM_MSAA_SAMPLES=0")
target_include_directories(rmlui_backend_GLFW_GL3 INTERFACE glad/include RmlUi)

# Header only
target_include_directories(exts INTERFACE header_only)

target_link_libraries(
  exts
    INTERFACE 
        glfw
        ${OPENGL_LIBRARIES}
        glm
        imgui
        BulletDynamics
        BulletCollision
        LinearMath
        yaml-cpp
        imguizmo
        RmlUi::RmlUi
        rmlui_backend_GLFW_GL3
      )

if(NOT DEFINED EMSCRIPTEN)
  target_link_libraries(exts INTERFACE glad)

endif()
