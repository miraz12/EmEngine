cmake_minimum_required(VERSION 3.10)
project(EmEngineTests)

# exts/CMakeLists.txt force-sets CMAKE_RUNTIME_OUTPUT_DIRECTORY to
# resources/lib via the cache. Override it here so test binaries land in the
# normal build directory instead.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "" FORCE)

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

# ---------------------------------------------------------------------------
# Visual regression tests
# ---------------------------------------------------------------------------

option(GENERATE_REFS "Write reference images instead of diffing (re-baselining)" OFF)

add_executable(visual_tests visual_test_main.cpp test_engine_shaders.cpp)

target_include_directories(visual_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/exts/header_only
    ${CMAKE_SOURCE_DIR}/exts/glad/include
    ${CMAKE_SOURCE_DIR}/exts/glfw/include
)

set_property(TARGET visual_tests PROPERTY CXX_STANDARD 23)
set_property(TARGET visual_tests PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(visual_tests Engine exts)

target_include_directories(visual_tests SYSTEM
    PUBLIC ${CMAKE_SOURCE_DIR}/src/Engine)

if(GENERATE_REFS)
    target_compile_definitions(visual_tests PRIVATE GENERATE_REFS)
endif()

add_test(NAME VisualTests COMMAND visual_tests)
set_tests_properties(VisualTests PROPERTIES
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    ENVIRONMENT "REF_IMAGE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/reference_images;SCENE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/scenes"
)
