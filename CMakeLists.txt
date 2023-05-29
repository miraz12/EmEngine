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
                        $<$<CONFIG:Debug>:
                            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                                    -pedantic -Wall -Wextra -g3 -O0>
                                    $<$<CXX_COMPILER_ID:MSVC>:
                                    /W4;>>)
target_compile_options(${PROJECT_NAME} PRIVATE
                        $<$<CONFIG:Release>:
                            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
                                    -pedantic -Wall -Wextra -O3 -Werror>
                                    $<$<CXX_COMPILER_ID:MSVC>:
                                    /W4;>>)

# target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)

if (DEFINED EMSCRIPTEN)
configure_file(${CMAKE_SOURCE_DIR}resources/js/HandleTouchMove.js ${CMAKE_CURRENT_BINARY_DIR}/HandleTouchMove.js COPYONLY)

target_compile_options(${PROJECT_NAME} PRIVATE -pedantic -Wall -Wextra -g)

set_target_properties(${PROJECT_NAME}
        PROPERTIES SUFFIX ".html"
        LINK_FLAGS " --bind -sFULL_ES3 -sUSE_GLFW=3  
                     -sWASM=1 -sMAX_WEBGL_VERSION=2 -sALLOW_MEMORY_GROWTH 
                     -gsource-map --source-map-base http://localhost:8000/build/ 
                     --preload-file ../resources
                     --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/HandleTouchMove.js")
endif (DEFINED EMSCRIPTEN)
