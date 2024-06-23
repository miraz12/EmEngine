cmake_minimum_required(VERSION 3.16)

# Add source files
file(GLOB_RECURSE ENGINE_FILES ${CMAKE_SOURCE_DIR}/src/Engine/*.cpp
     ${CMAKE_SOURCE_DIR}/src/Engine/*.hpp)

# Add source files as libraries
add_library(Engine ${ENGINE_FILES})
target_include_directories(Engine SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Engine SYSTEM
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(Engine SYSTEM
                           PUBLIC ${CMAKE_SOURCE_DIR}/src/Game/include)

add_dependencies(Engine Game)
target_link_libraries(Engine PRIVATE Game)
if(NOT DEFINED EMSCRIPTEN)
  target_link_directories(Engine INTERFACE ${CMAKE_SOURCE_DIR}/resources/lib)
else()
  target_link_directories(Engine INTERFACE
                          ${CMAKE_SOURCE_DIR}/resources/libwasm)
endif()

target_link_libraries(Engine PRIVATE exts)
target_precompile_headers(Engine PRIVATE engine_pch.hpp)

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
