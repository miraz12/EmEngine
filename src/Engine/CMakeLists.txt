cmake_minimum_required(VERSION 3.16)

# Add source files
file(GLOB_RECURSE ENGINE_FILES ${CMAKE_SOURCE_DIR}/src/Engine/*.cpp
     ${CMAKE_SOURCE_DIR}/src/Engine/*.hpp)

# Add source files as libraries
if(DEFINED EMSCRIPTEN)
  add_library(
    Engine STATIC
    ${ENGINE_FILES} $<TARGET_OBJECTS:yaml-cpp> $<TARGET_OBJECTS:imgui>
    $<TARGET_OBJECTS:BulletCollision> $<TARGET_OBJECTS:BulletDynamics>
    $<TARGET_OBJECTS:LinearMath>)
else()
  add_library(
    Engine STATIC
    ${ENGINE_FILES}
    $<TARGET_OBJECTS:yaml-cpp>
    $<TARGET_OBJECTS:imgui>
    $<TARGET_OBJECTS:imguizmo>
    $<TARGET_OBJECTS:BulletCollision>
    $<TARGET_OBJECTS:BulletDynamics>
    $<TARGET_OBJECTS:LinearMath>
    $<TARGET_OBJECTS:glad>
    $<TARGET_OBJECTS:glfw>)
endif(DEFINED EMSCRIPTEN)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(
  Engine
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib")

target_link_libraries(Engine PUBLIC exts)
target_precompile_headers(Engine PUBLIC engine_pch.hpp)
target_compile_definitions(Engine PUBLIC GLM_FORCE_SILENT_WARNINGS=1)
set_property(TARGET Engine PROPERTY CXX_STANDARD 20)
set_property(TARGET Engine PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_options(
  Engine
  PRIVATE
    $<$<CONFIG:Debug>:
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -pedantic
    -Wall
    -Wextra
    -g3
    -O0>
    $<$<CXX_COMPILER_ID:MSVC>:
    /W4
    /MT;>>)
target_compile_options(
  Engine
  PRIVATE
    $<$<CONFIG:Release>:
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -pedantic
    -Wall
    -Wextra
    -O3
    -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:
    /W4
    /MT;>>)
