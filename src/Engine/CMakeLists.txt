cmake_minimum_required(VERSION 3.16)

# Engine source files
set(ENGINE_FILES
  Core.cpp
  Core.hpp
  engine_pch.hpp
  GameStateManager.cpp
  GameStateManager.hpp
  Gui.cpp
  Gui.hpp
  InputManager.cpp
  InputManager.hpp
  MapLoader.cpp
  MapLoader.hpp
  ResourceManager.cpp
  ResourceManager.hpp
  SceneLoader.cpp
  SceneLoader.hpp
  Singleton.hpp
  UIManager.cpp
  UIManager.hpp
  Window.cpp
  Window.hpp

  # ECS
  ECS/ECSManager.cpp
  ECS/ECSManager.hpp
  ECS/ComponentPool.hpp
  ECS/Components/AnimationComponent.hpp
  ECS/Components/CameraComponent.hpp
  ECS/Components/Component.hpp
  ECS/Components/DebugComponent.hpp
  ECS/Components/GraphicsComponent.hpp
  ECS/Components/LightingComponent.hpp
  ECS/Components/ParticlesComponent.hpp
  ECS/Components/PhysicsComponent.cpp
  ECS/Components/PhysicsComponent.hpp
  ECS/Components/PositionComponent.hpp
  ECS/Systems/AnimationSystem.cpp
  ECS/Systems/AnimationSystem.hpp
  ECS/Systems/CameraSystem.cpp
  ECS/Systems/CameraSystem.hpp
  ECS/Systems/GraphicsSystem.cpp
  ECS/Systems/GraphicsSystem.hpp
  ECS/Systems/ParticleSystem.cpp
  ECS/Systems/ParticleSystem.hpp
  ECS/Systems/PhysicsLayers.hpp
  ECS/Systems/PhysicsSystem.cpp
  ECS/Systems/PhysicsSystem.hpp
  ECS/Systems/PositionSystem.cpp
  ECS/Systems/PositionSystem.hpp
  ECS/Systems/System.hpp

  # Graphics
  Graphics/Backends/GLES3/Device.cpp
  Graphics/Backends/GLES3/Device.hpp
  Graphics/Backends/GLES3/Resources.hpp
  Graphics/CommandBuffer.cpp
  Graphics/CommandBuffer.hpp
  Graphics/GraphicsDevice.cpp
  Graphics/GraphicsDevice.hpp
  Graphics/GraphicsTypes.hpp
  Graphics/Handle.hpp
  Graphics/RenderResources.cpp
  Graphics/RenderResources.hpp
  Graphics/Resources/Buffer.hpp
  Graphics/Resources/Framebuffer.hpp
  Graphics/Resources/Pipeline.hpp
  Graphics/Resources/Renderbuffer.hpp
  Graphics/Resources/Sampler.hpp
  Graphics/Resources/Shader.hpp
  Graphics/Resources/Texture.hpp
  Graphics/UBOStructs.hpp

  # Objects
  Objects/Cube.cpp
  Objects/Cube.hpp
  Objects/GltfObject.cpp
  Objects/GltfObject.hpp
  Objects/GraphicsObject.cpp
  Objects/GraphicsObject.hpp
  Objects/Heightmap.cpp
  Objects/Heightmap.hpp
  Objects/Line.cpp
  Objects/Line.hpp
  Objects/Point.cpp
  Objects/Point.hpp
  Objects/Quad.cpp
  Objects/Quad.hpp

  # Rendering
  Rendering/Animation.cpp
  Rendering/Animation.hpp
  Rendering/DebugDrawer.cpp
  Rendering/DebugDrawer.hpp
  Rendering/Material.cpp
  Rendering/Material.hpp
  Rendering/Mesh.hpp
  Rendering/Node.hpp
  Rendering/Primitive.cpp
  Rendering/Primitive.hpp
  Rendering/Skin.hpp

  # Render Passes
  RenderPasses/BloomPass.cpp
  RenderPasses/BloomPass.hpp
  RenderPasses/CubeMapPass.cpp
  RenderPasses/CubeMapPass.hpp
  RenderPasses/DebugPass.cpp
  RenderPasses/DebugPass.hpp
  RenderPasses/FrameGraph.cpp
  RenderPasses/FrameGraph.hpp
  RenderPasses/FxaaPass.cpp
  RenderPasses/FxaaPass.hpp
  RenderPasses/GeometryPass.cpp
  RenderPasses/GeometryPass.hpp
  RenderPasses/LightingUtil.hpp
  RenderPasses/LightPass.cpp
  RenderPasses/LightPass.hpp
  RenderPasses/ParticlePass.cpp
  RenderPasses/ParticlePass.hpp
  RenderPasses/RenderPass.cpp
  RenderPasses/RenderPass.hpp
  RenderPasses/RenderUtil.hpp
  RenderPasses/ShadowPass.cpp
  RenderPasses/ShadowPass.hpp

  # Types
  Types/LightTypes.hpp

  # Public API
  include/engine_api.hpp
)

# Add source files as libraries
if(DEFINED EMSCRIPTEN)
  add_library(
    Engine STATIC
    ${ENGINE_FILES} $<TARGET_OBJECTS:yaml-cpp> $<TARGET_OBJECTS:imgui>
    $<TARGET_OBJECTS:Jolt>)

  set_target_properties(
    Engine
    PROPERTIES
      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib/wasm"
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib/wasm"
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib/wasm"
  )
else()
  add_library(
    Engine STATIC
    ${ENGINE_FILES}
    $<TARGET_OBJECTS:yaml-cpp>
    $<TARGET_OBJECTS:imgui>
    $<TARGET_OBJECTS:imguizmo>
    $<TARGET_OBJECTS:Jolt>
    $<TARGET_OBJECTS:glad>
    $<TARGET_OBJECTS:glfw>)

   set_target_properties(
     Engine
     PROPERTIES
       ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
       LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
       RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/resources/lib"
   )
endif(DEFINED EMSCRIPTEN)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_link_libraries(Engine PUBLIC exts)
target_precompile_headers(Engine PUBLIC engine_pch.hpp)
target_compile_definitions(Engine PUBLIC GLM_FORCE_SILENT_WARNINGS=1)
set_property(TARGET Engine PROPERTY CXX_STANDARD 23)
set_property(TARGET Engine PROPERTY CXX_STANDARD_REQUIRED ON)

target_compile_options(
  Engine
  PRIVATE
    $<$<CONFIG:Debug>:
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -pedantic
    -Wall
    -Wextra
    -g3
    -O0
    -fPIC>
    $<$<CXX_COMPILER_ID:MSVC>:
    /W4
    /MT;>>)
target_compile_options(
  Engine
  PRIVATE
    $<$<CONFIG:Release>:
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -pedantic
    -Wall
    -Wextra
    -O3
    # -Werror
    -fPIC>
    $<$<CXX_COMPILER_ID:MSVC>:
    /W4
    /MT;>>)
