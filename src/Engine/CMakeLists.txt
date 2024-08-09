cmake_minimum_required(VERSION 3.16)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/resources/lib/)

# Add source files
file(GLOB_RECURSE ENGINE_FILES ${CMAKE_SOURCE_DIR}/src/Engine/*.cpp
     ${CMAKE_SOURCE_DIR}/src/Engine/*.hpp)

# Add source files as libraries
if(DEFINED EMSCRIPTEN)
  add_library(Engine STATIC ${ENGINE_FILES} $<TARGET_OBJECTS:yaml-cpp> $<TARGET_OBJECTS:imgui> $<TARGET_OBJECTS:BulletCollision> $<TARGET_OBJECTS:BulletDynamics> $<TARGET_OBJECTS:LinearMath>)
else()
  add_library(Engine SHARED ${ENGINE_FILES})
endif(DEFINED EMSCRIPTEN)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Engine PUBLIC exts)
target_precompile_headers(Engine PUBLIC engine_pch.hpp)

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
    /W4;>>)
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
    /W4;>>)

set_target_properties(
  Engine
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED ON
             CMAKE_CXX_EXTENSIONS OFF)
target_compile_definitions(Engine PUBLIC GLM_FORCE_SILENT_WARNINGS=1)
