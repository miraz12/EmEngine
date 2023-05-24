cmake_minimum_required(VERSION 3.16)

# Add source files
file(GLOB_RECURSE GAME_FILES
    ${CMAKE_SOURCE_DIR}/src/Game/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Game/*.hpp)

# Add source files as libraries
add_library(Game ${GAME_FILES})
target_include_directories(Game SYSTEM PUBLIC ${CMAKE_SOURCE_DIR}/src/Game)
target_link_libraries(Game PUBLIC exts Engine)
target_compile_options(Game PRIVATE
                        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                        -pedantic -Wall -Wextra -g3 -O0>
                        $<$<CXX_COMPILER_ID:MSVC>:
                        /W4;>)

set_target_properties(Game PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CMAKE_CXX_EXTENSIONS OFF)
target_compile_definitions(Game PUBLIC GLM_FORCE_SILENT_WARNINGS=1)

