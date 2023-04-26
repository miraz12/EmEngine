cmake_minimum_required(VERSION 3.12)
project(emEngine)
# Generate compile_commands.json for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EMCC_DEBUG 1)
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)


# Add source files
file(GLOB_RECURSE GAME_FILES
    ${CMAKE_SOURCE_DIR}/src/Game/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Game/*.hpp)
# Add external libs
add_subdirectory(exts)
add_subdirectory(src/Engine)
add_subdirectory(src/Game)

# Creaty executable
add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/src/main.cpp)

# Link libraries
target_link_libraries(${PROJECT_NAME} PUBLIC Engine Game)
target_compile_options(${PROJECT_NAME} PRIVATE
                        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                        -pedantic -Wall -Wextra -Werror>
                        $<$<CXX_COMPILER_ID:MSVC>:
                        /W4;>)
# target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

if (DEFINED EMSCRIPTEN)
set_target_properties(${PROJECT_NAME}
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS " --bind -sFULL_ES3 -s USE_GLFW=3 -s WASM=1 -s ALLOW_MEMORY_GROWTH --preload-file ../resources ")
        #LINK_FLAGS " --bind -sFULL_ES3 -s USE_GLFW=3 -s WASM=1 -s ALLOW_MEMORY_GROWTH -gsource-map --source-map-base http://localhost:8000/ --preload-file ../resources ")
endif (DEFINED EMSCRIPTEN)
