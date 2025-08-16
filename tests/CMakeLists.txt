cmake_minimum_required(VERSION 3.10)
project(EmEngineTests)

# Add source files
add_executable(emengine_tests component_tests.cpp ecs_tests.cpp core_tests.cpp
                              scene_tests.cpp math_tests.cpp)

target_precompile_headers(emengine_tests PUBLIC
                          ${CMAKE_SOURCE_DIR}/src/Engine/engine_pch.hpp)

# Link libraries
target_link_libraries(emengine_tests gtest gmock gtest_main Engine exts)

target_include_directories(emengine_tests SYSTEM
                           PUBLIC ${CMAKE_SOURCE_DIR}/src/Engine)

# Add tests
add_test(NAME ComponentTests COMMAND emengine_tests
                                     --gtest_filter=*ComponentTest*)
add_test(NAME ECSTests COMMAND emengine_tests --gtest_filter=*ECSManagerTest*)
add_test(NAME CoreTests COMMAND emengine_tests --gtest_filter=*CoreTest*)
add_test(NAME SceneTests COMMAND emengine_tests --gtest_filter=*SceneTest*)
add_test(NAME MathTests COMMAND emengine_tests --gtest_filter=*MathTest*)
add_test(NAME SingletonTests COMMAND emengine_tests
                                     --gtest_filter=*SingletonTest*)
add_test(NAME InputManagerTests COMMAND emengine_tests
                                        --gtest_filter=*InputManagerTest*)
add_test(NAME GLMTests COMMAND emengine_tests
                               --gtest_filter=*GLMIntegrationTest*)
add_test(NAME EntityStressTests COMMAND emengine_tests
                                        --gtest_filter=*EntityStressTest*)
