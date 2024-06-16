cmake_minimum_required(VERSION 3.10)
project(EmEngineTests)

# Add source files
add_executable(emengine_tests ecs_tests.cpp)
target_precompile_headers(emengine_tests PUBLIC
                          ${CMAKE_SOURCE_DIR}/src/Engine/engine_pch.hpp)

# Link libraries
target_link_libraries(emengine_tests gtest gmock gtest_main Engine exts)

target_include_directories(emengine_tests SYSTEM
                           PUBLIC ${CMAKE_SOURCE_DIR}/src/Engine)

# Add tests
add_test(NAME ECSManagerTests COMMAND emengine_tests)
