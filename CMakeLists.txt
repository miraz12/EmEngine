cmake_minimum_required(VERSION 3.12)
project(emEngine)
# Generate compile_commands.json for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EMCC_DEBUG 1)


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

if (DEFINED EMSCRIPTEN)
set_target_properties(${PROJECT_NAME}
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS " --bind -sFULL_ES3 -s USE_GLFW=3 -s WASM=1 -gsource-map=http://localhost:8000/ -s NO_FILESYSTEM=1")
endif (DEFINED EMSCRIPTEN)
