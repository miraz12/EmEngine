cmake_minimum_required(VERSION 3.12)
project(emEngine)
# Generate compile_commands.json for LSP
if(LINUX)
  set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
        ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  endif()
endif()
# set(EMCC_DEBUG 1)

# Add external libs
add_subdirectory(exts)
add_subdirectory(src/Engine)
add_subdirectory(src/Game)

include_directories(${CMAKE_SYSROOT}/include)
include_directories(${CMAKE_SYSROOT}/include/c++/v1)

if(NOT DEFINED EMSCRIPTEN)
  # Enable testing
  enable_testing()
  add_subdirectory(tests)
endif()
