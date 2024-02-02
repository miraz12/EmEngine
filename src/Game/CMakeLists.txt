cmake_minimum_required(VERSION 3.16)

cmake_minimum_required(VERSION 3.10)

# Project name
project(v_game_lib)

# Specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Set the output directory for the shared library
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/resources/lib)

# Custom command to compile the V source code into a shared library
add_custom_command(
  OUTPUT ${LIBRARY_OUTPUT_PATH}/libv_example.so
  COMMAND v -cg -cc gcc -shared -o ${LIBRARY_OUTPUT_PATH}/libv_example.so
          ${PROJECT_SOURCE_DIR}/Game.v
  DEPENDS ${PROJECT_SOURCE_DIR}/Game.v
  COMMENT "Building V shared library")

# Custom target that depends on the output of the custom command
add_custom_target(v_game_lib ALL DEPENDS ${LIBRARY_OUTPUT_PATH}/libv_example.so)

# Example of how to link the V shared library with another C project
# add_executable(c_example main.c) add_dependencies(c_example v_game_lib)
# target_link_libraries(c_example ${LIBRARY_OUTPUT_PATH}/libv_example.so)
