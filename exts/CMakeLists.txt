add_library(exts INTERFACE null.cpp)

function(disable_warnings target)
  if(LINUX)
    target_compile_options(${target} PRIVATE -Wno-everything)
  elseif(WIN32)
    target_compile_options(${target} PRIVATE /W0)
  endif()
endfunction()

if(NOT DEFINED EMSCRIPTEN)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib" CACHE PATH "" FORCE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib" CACHE PATH "" FORCE)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib" CACHE PATH "" FORCE)
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

else()
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib/wasm" CACHE PATH "" FORCE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib/wasm" CACHE PATH "" FORCE)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib/wasm" CACHE PATH "" FORCE)
  add_library(glfw INTERFACE)
  target_include_directories(glfw INTERFACE ${CMAKE_SYSROOT}/include)
endif()

# GLAD
add_library(glad glad/src/glad.c glad/include/glad/glad.h)
target_include_directories(glad PUBLIC glad/include)
disable_warnings(glad)

# Jolt Physics
set(TARGET_HELLO_WORLD OFF CACHE BOOL "" FORCE)
set(TARGET_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(TARGET_PERFORMANCE_TEST OFF CACHE BOOL "" FORCE)
set(TARGET_SAMPLES OFF CACHE BOOL "" FORCE)
set(TARGET_VIEWER OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)
if(DEFINED EMSCRIPTEN)
  set(JPH_USE_SSE OFF CACHE BOOL "" FORCE)
  set(JPH_USE_AVX OFF CACHE BOOL "" FORCE)
  set(JPH_USE_AVX2 OFF CACHE BOOL "" FORCE)
  set(JPH_USE_AVX512 OFF CACHE BOOL "" FORCE)
  set(JPH_USE_LZCNT OFF CACHE BOOL "" FORCE)
  set(JPH_USE_TZCNT OFF CACHE BOOL "" FORCE)
  set(JPH_USE_F16C OFF CACHE BOOL "" FORCE)
  set(JPH_USE_FMADD OFF CACHE BOOL "" FORCE)
endif()
set(DEBUG_RENDERER_IN_DEBUG_AND_RELEASE OFF CACHE BOOL "" FORCE)
set(DEBUG_RENDERER_IN_DISTRIBUTION OFF CACHE BOOL "" FORCE)
add_subdirectory(JoltPhysics/Build)
target_include_directories(exts SYSTEM INTERFACE JoltPhysics)

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
disable_warnings(imgui)

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
disable_warnings(imguizmo)

# FreeType
add_subdirectory(freetype)

add_library(Freetype::Freetype ALIAS freetype)

# RmlUi
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "" FORCE)
set(RMLUI_BACKEND GLFW_GL3 CACHE BOOL "" FORCE)
set(RMLUI_SHELL ON CACHE BOOL "" FORCE)

add_subdirectory(RmlUi)

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
        Jolt
        yaml-cpp
        imguizmo
        RmlUi::RmlUi
        rmlui_backend_GLFW_GL3
      )

if(NOT DEFINED EMSCRIPTEN)
  target_link_libraries(exts INTERFACE glad)

endif()
