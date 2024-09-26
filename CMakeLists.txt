cmake_minimum_required(VERSION 3.12)
project(emEngine)
# Generate compile_commands.json for LSP
if(UNIX)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
        ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  endif()
endif()
# set(EMCC_DEBUG 1) set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

# Add external libs
add_subdirectory(exts)
add_subdirectory(src/Engine)
add_subdirectory(src/Game)

include_directories(${CMAKE_SYSROOT}/include)
include_directories(${CMAKE_SYSROOT}/include/c++/v1)

# if(NOT DEFINED EMSCRIPTEN) Enable testing enable_testing()
# add_subdirectory(tests) endif()

# Creaty executable add_executable(${PROJECT_NAME}
# ${CMAKE_SOURCE_DIR}/src/main.cpp) target_precompile_headers(${PROJECT_NAME}
# PRIVATE ${CMAKE_SOURCE_DIR}/src/Engine/engine_pch.hpp)

# add_dependencies(${PROJECT_NAME} Game) target_link_libraries(${PROJECT_NAME}
# PUBLIC Game)

# Link libraries target_link_libraries(${PROJECT_NAME} PUBLIC Engine exts)
# target_compile_options( ${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:
# $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
# -pedantic -Wall -Wextra -g3 -O0> $<$<CXX_COMPILER_ID:MSVC>: /W4;>>)

# target_compile_options( ${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:
# $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
# -pedantic -Wall -Wextra -O3 -Werror -fno-exceptions -flto -ffast-math >
# $<$<CXX_COMPILER_ID:MSVC>: /W4;>>)

# target_link_options( ${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:
# $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
# -flto > $<$<CXX_COMPILER_ID:MSVC>: >>)

# target_link_options(${PROJECT_NAME} PRIVATE -static-libsan)
# set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
# "${CMAKE_SOURCE_DIR}")
set_property(GLOBAL PROPERTY CXX_STANDARD 17)

if(DEFINED EMSCRIPTEN)
  # target_compile_options( ${PROJECT_NAME} PRIVATE -pedantic -Wall -Wextra -O2
  # -Werror -fno-exceptions -flto -ffast-math)

  # set_target_properties( ${PROJECT_NAME} PROPERTIES SUFFIX ".html" LINK_FLAGS
  # "../../resources --bind -sFULL_ES2 -sUSE_GLFW=3 -sWASM=1
  # -sMAX_WEBGL_VERSION=2 -sALLOW_MEMORY_GROWTH --preload-file ../../resources
  # -sFORCE_FILESYSTEM" )
endif(DEFINED EMSCRIPTEN)
