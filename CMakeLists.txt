cmake_minimum_required(VERSION 3.12)

# Setup emsdk before project() is called
set(EMSDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/exts/emsdk")

if(EMSCRIPTEN)
    # Check if emsdk needs initialization
    if(NOT EXISTS "${EMSDK_PATH}/upstream/emscripten")
        message(STATUS "Installing emsdk...")
        execute_process(
            COMMAND ./emsdk install latest
            WORKING_DIRECTORY ${EMSDK_PATH}
            RESULT_VARIABLE EMSDK_INSTALL_RESULT
        )

        execute_process(
            COMMAND ./emsdk activate latest
            WORKING_DIRECTORY ${EMSDK_PATH}
            RESULT_VARIABLE EMSDK_ACTIVATE_RESULT
        )

        if(NOT EMSDK_INSTALL_RESULT EQUAL 0 OR NOT EMSDK_ACTIVATE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to setup emsdk")
        endif()
    endif()

    # Set the toolchain file
    message(STATUS "Use emsdk...")
    set(CMAKE_TOOLCHAIN_FILE "${EMSDK_PATH}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
endif()

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
