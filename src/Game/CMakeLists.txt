cmake_minimum_required(VERSION 3.10)

if(NOT DEFINED EMSCRIPTEN)

  # Specify the C standard
  set(CMAKE_C_STANDARD 99)
  set(CMAKE_C_STANDARD_REQUIRED True)

  # Set the output directory for the shared library
  set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/resources/lib)

  # Custom command to compile the V source code into a shared library
  add_custom_command(
    OUTPUT ${LIBRARY_OUTPUT_PATH}/libv_game.so
    COMMAND v -cc gcc -shared -o ${LIBRARY_OUTPUT_PATH}/libv_game.so
            ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building V shared library")

  # Custom target that depends on the output of the custom command
  add_custom_target(v_game_lib ALL DEPENDS ${LIBRARY_OUTPUT_PATH}/libv_game.so)

else(NOT DEFINED EMSCRIPTEN)

  # Add source files Custom command to compile the V source code into a shared
  # library
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Game.c
    COMMAND v -d no_main -o ${CMAKE_CURRENT_SOURCE_DIR}/Game.c -os
            wasm32_emscripten -w ${CMAKE_CURRENT_SOURCE_DIR} -rpath=.
    DEPENDS Game.v
    COMMENT "Building V to C")

  add_custom_target(v_game_c ALL DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Game.c)

  file(GLOB_RECURSE GAME_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c
       ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

  # Add source files as libraries
  add_library(Game STATIC ${GAME_FILES})
  add_dependencies(Game v_game_c)

  # target_include_directories( Game SYSTEM PUBLIC
  # "/nix/store/xppqwbmryhif3hkfn305ihq77kmjkgrf-boehm-gc-8.2.2-dev/include"
  # "/nix/store/pldw5bd9yc7da0mjh01zd9azbfshpd2c-glibc-2.38-23-dev/include")
  target_include_directories(Game SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  target_include_directories(Game SYSTEM
                             PUBLIC ${CMAKE_SOURCE_DIR}/src/Engine/include)

  set_target_properties(Game PROPERTIES LINKER_LANGUAGE C)

  # target_compile_options( Game PRIVATE $<$<CONFIG:Debug>:
  # $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
  # -pedantic -Wall -Wextra -g3 -O0> $<$<CXX_COMPILER_ID:MSVC>: /W4;>>)
  # target_compile_options( Game PRIVATE $<$<CONFIG:Release>:
  # $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
  # -pedantic -Wall -Wextra -O3 -Werror> $<$<CXX_COMPILER_ID:MSVC>: /W4;>>)

  # set_target_properties( Game PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED
  # ON CMAKE_CXX_EXTENSIONS OFF)

endif(NOT DEFINED EMSCRIPTEN)
