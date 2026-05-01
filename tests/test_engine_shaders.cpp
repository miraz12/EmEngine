#include "visual_test_harness.h"
#include "visual_test_utils.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <ECS/ECSManager.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <Graphics/UBOStructs.hpp>
#include <RenderPasses/FrameGraph.hpp>
#include <SceneLoader.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <filesystem>
#include <print>
#include <string>

// Forward declaration from visual_test_main.cpp
bool
registerVisualTest(const std::string& name,
                   std::function<bool(VisualTestHarness&)> func);

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string
getRefDir()
{
  const char* refEnv = std::getenv("REF_IMAGE_DIR");
  return refEnv
           ? refEnv
           : (std::filesystem::current_path() / "tests" / "reference_images")
               .string();
}

static std::string
getSceneDir()
{
  const char* sceneEnv = std::getenv("SCENE_DIR");
  return sceneEnv
           ? sceneEnv
           : (std::filesystem::current_path() / "tests" / "scenes").string();
}

static bool
captureAndCompare(VisualTestHarness& harness, const std::string& testName)
{
  std::string refPath = getRefDir() + "/" + testName + ".png";
  std::string actualPath = "actual/" + testName + ".png";

#ifdef GENERATE_REFS
  if (!harness.captureToFile(refPath)) {
    std::println(stderr, "  [{}] Failed to write reference image", testName);
    return false;
  }
  std::println("  [{}] Reference image written to: {}", testName, refPath);
  return true;
#else
  if (!harness.captureToFile(actualPath)) {
    std::println(stderr, "  [{}] Failed to capture actual image", testName);
    return false;
  }

  DiffResult diff = compareImages(refPath, actualPath);
  std::print("  [{}] diffPercent={}% maxDiff={}",
             testName,
             diff.diffPercent,
             diff.maxDiff);

  if (diff.passed) {
    std::println(" -> OK");
  } else {
    std::println(" -> EXCEEDED THRESHOLD");
  }

  return diff.passed;
#endif
}

// Loads a scene YAML, runs one FrameGraph frame, and compares the result.
static bool
runSceneTest(VisualTestHarness& harness,
             const std::string& sceneFile,
             const std::string& testName)
{
  const u32 w = static_cast<u32>(harness.getWidth());
  const u32 h = static_cast<u32>(harness.getHeight());

  auto& ecs = ECSManager::getInstance();
  ecs.reset();

  std::string scenePath = getSceneDir() + "/" + sceneFile;
  SceneLoader::getInstance().init(scenePath.c_str());

  auto& fg = FrameGraph::getInstance();
  fg.setViewport(w, h);
  fg.draw(ecs);

  glFinish();
  return captureAndCompare(harness, testName);
}

// ---------------------------------------------------------------------------
// Test 1: Debug Lines — uses debugLine.vert/frag with Camera UBO
//
// Renders colored line segments forming an axis cross and a diamond shape
// using the engine's debug line shaders with a perspective camera.
// ---------------------------------------------------------------------------

static bool
debugLinesTest(VisualTestHarness& harness)
{
  auto& device = gfx::GraphicsDevice::getInstance();
  auto& resources = gfx::RenderResources::getInstance();

  // Load engine debug line shaders
  gfx::ShaderId shader =
    resources.loadShaderProgram("test_debugLine",
                                "resources/Shaders/debugLine.vert",
                                "resources/Shaders/debugLine.frag");
  if (!device.isValid(shader)) {
    std::println(stderr, "  [debug_lines] Shader load failed");
    return false;
  }

  // Bind Camera UBO to shader
  resources.bindShaderUniformBlock(
    shader, "CameraData", gfx::UBOBinding::Camera);

  // Set up camera looking at origin
  auto& cam = resources.getCameraUBO();
  cam.viewMatrix = glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f),  // eye
                               glm::vec3(0.0f, 0.0f, 0.0f),  // center
                               glm::vec3(0.0f, 1.0f, 0.0f)); // up
  cam.projMatrix = glm::perspective(glm::radians(45.0f),
                                    static_cast<float>(harness.getWidth()) /
                                      static_cast<float>(harness.getHeight()),
                                    0.1f,
                                    100.0f);
  cam.viewProjMatrix = cam.projMatrix * cam.viewMatrix;
  cam.cameraPosition = glm::vec4(0.0f, 2.0f, 5.0f, 0.0f);
  resources.flushCameraUBO();

  // Vertex data: position (vec3) + color (vec3), lines topology
  struct LineVertex
  {
    glm::vec3 position;
    glm::vec3 color;
  };

  // Axis lines + a diamond shape
  LineVertex vertices[] = {
    // X axis (red)
    { { -2.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 2.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    // Y axis (green)
    { { 0.0f, -2.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, 2.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    // Z axis (blue)
    { { 0.0f, 0.0f, -2.0f }, { 0.0f, 0.0f, 1.0f } },
    { { 0.0f, 0.0f, 2.0f }, { 0.0f, 0.0f, 1.0f } },
    // Diamond (yellow)
    { { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } },
    { { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } },
    { { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } },
    { { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } },
    { { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } },
    { { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } },
    { { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } },
  };

  gfx::BufferCreateInfo bufInfo{};
  bufInfo.size = sizeof(vertices);
  bufInfo.usage = gfx::BufferUsage::Vertex;
  bufInfo.initialData = vertices;
  bufInfo.debugName = "test_debug_lines_vbo";
  gfx::BufferId vbo = device.createBuffer(bufInfo);

  gfx::VertexBinding binding{};
  binding.binding = 0;
  binding.stride = sizeof(LineVertex);
  binding.perInstance = false;

  gfx::VertexAttribute attrs[2]{};
  attrs[0].location = 0;
  attrs[0].binding = 0;
  attrs[0].offset = offsetof(LineVertex, position);
  attrs[0].format = gfx::PixelFormat::RGB32F;

  attrs[1].location = 1;
  attrs[1].binding = 0;
  attrs[1].offset = offsetof(LineVertex, color);
  attrs[1].format = gfx::PixelFormat::RGB32F;

  gfx::VertexArrayCreateInfo vaoInfo{};
  vaoInfo.vertexBindings = std::span(&binding, 1);
  vaoInfo.vertexAttributes = std::span(attrs, 2);
  vaoInfo.topology = gfx::PrimitiveTopology::Lines;
  vaoInfo.vertexCount = std::size(vertices);
  vaoInfo.debugName = "test_debug_lines_vao";

  gfx::VertexArrayId vao = device.createVertexArray(vaoInfo);

  // Render
  device.bindFramebuffer(device.getDefaultFramebuffer());
  device.setViewport(0, 0, harness.getWidth(), harness.getHeight());
  device.setDepthTest(false);
  device.clearColor(std::nullopt, glm::vec4(0.05f, 0.05f, 0.1f, 1.0f));

  device.bindShaderProgram(shader);
  device.setLineWidth(2.0f);

  // Set modelMatrix uniform to identity
  i32 modelLoc = device.getUniformLocation(shader, "modelMatrix");
  device.setUniformMat4(modelLoc, glm::mat4(1.0f));

  device.drawVertexArray(vao, vbo);

  glFinish();
  return captureAndCompare(harness, "debug_lines");
}

// ---------------------------------------------------------------------------
// Test 2: FXAA Post-Process — uses quad.vert + Fxaa.frag
//
// Renders a procedural scene to a texture (high-contrast edges), then
// applies the engine's FXAA shader via fullscreen quad to test the
// post-processing pipeline with PostProcess UBO.
// ---------------------------------------------------------------------------

static bool
fxaaTest(VisualTestHarness& harness)
{
  auto& device = gfx::GraphicsDevice::getInstance();
  auto& resources = gfx::RenderResources::getInstance();

  const u32 w = static_cast<u32>(harness.getWidth());
  const u32 h = static_cast<u32>(harness.getHeight());

  // First, render a high-contrast scene to a texture using debug line shaders
  // (sharp colored lines create good FXAA test cases)
  gfx::ShaderId lineShader = resources.getShaderProgram("test_debugLine");
  if (!device.isValid(lineShader)) {
    lineShader =
      resources.loadShaderProgram("test_debugLine",
                                  "resources/Shaders/debugLine.vert",
                                  "resources/Shaders/debugLine.frag");
    resources.bindShaderUniformBlock(
      lineShader, "CameraData", gfx::UBOBinding::Camera);
  }

  // Create offscreen texture + framebuffer for the scene
  gfx::TextureCreateInfo sceneTexInfo{};
  sceneTexInfo.type = gfx::TextureType::Texture2D;
  sceneTexInfo.format = gfx::PixelFormat::RGBA8;
  sceneTexInfo.width = w;
  sceneTexInfo.height = h;
  sceneTexInfo.mipLevels = 1;
  sceneTexInfo.debugName = "test_fxaa_scene";
  gfx::TextureId sceneTex =
    resources.createTexture2D("test_fxaa_scene", sceneTexInfo);

  gfx::RenderbufferCreateInfo depthRboInfo{};
  depthRboInfo.format = gfx::PixelFormat::Depth24;
  depthRboInfo.width = w;
  depthRboInfo.height = h;
  depthRboInfo.debugName = "test_fxaa_depth";
  resources.createRenderbuffer("test_fxaa_depth", depthRboInfo);

  gfx::FramebufferId sceneFbo =
    resources.createBareFramebuffer("test_fxaa_scene_fbo");
  resources.setFramebufferAttachment(
    "test_fxaa_scene_fbo", 0, "test_fxaa_scene");
  resources.setFramebufferRenderbuffer("test_fxaa_scene_fbo",
                                       gfx::RenderbufferAttachment::Depth,
                                       "test_fxaa_depth");

  // Set up camera
  auto& cam = resources.getCameraUBO();
  cam.viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f),
                               glm::vec3(0.0f, 0.0f, 0.0f),
                               glm::vec3(0.0f, 1.0f, 0.0f));
  cam.projMatrix =
    glm::perspective(glm::radians(45.0f),
                     static_cast<float>(w) / static_cast<float>(h),
                     0.1f,
                     100.0f);
  cam.viewProjMatrix = cam.projMatrix * cam.viewMatrix;
  cam.cameraPosition = glm::vec4(0.0f, 0.0f, 4.0f, 0.0f);
  resources.flushCameraUBO();

  // Draw diagonal lines to scene FBO (creates aliased edges for FXAA to work
  // on)
  struct LineVertex
  {
    glm::vec3 position;
    glm::vec3 color;
  };

  LineVertex lineVerts[] = {
    // Diagonal lines at various angles (white on dark = high contrast edges)
    { { -2.0f, -2.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } },
    { { 2.0f, 1.5f, 0.0f }, { 1.0f, 1.0f, 1.0f } },
    { { -2.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 2.0f, -1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { -1.5f, -2.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 1.5f, 2.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { 0.0f, -2.0f, 0.0f }, { 0.0f, 0.5f, 1.0f } },
    { { 1.0f, 2.0f, 0.0f }, { 0.0f, 0.5f, 1.0f } },
  };

  gfx::BufferCreateInfo lineBufInfo{};
  lineBufInfo.size = sizeof(lineVerts);
  lineBufInfo.usage = gfx::BufferUsage::Vertex;
  lineBufInfo.initialData = lineVerts;
  lineBufInfo.debugName = "test_fxaa_lines_vbo";
  gfx::BufferId lineVbo = device.createBuffer(lineBufInfo);

  gfx::VertexBinding lineBinding{};
  lineBinding.binding = 0;
  lineBinding.stride = sizeof(LineVertex);
  lineBinding.perInstance = false;

  gfx::VertexAttribute lineAttrs[2]{};
  lineAttrs[0] = {
    0, 0, offsetof(LineVertex, position), gfx::PixelFormat::RGB32F
  };
  lineAttrs[1] = {
    1, 0, offsetof(LineVertex, color), gfx::PixelFormat::RGB32F
  };

  gfx::VertexArrayCreateInfo lineVaoInfo{};
  lineVaoInfo.vertexBindings = std::span(&lineBinding, 1);
  lineVaoInfo.vertexAttributes = std::span(lineAttrs, 2);
  lineVaoInfo.topology = gfx::PrimitiveTopology::Lines;
  lineVaoInfo.vertexCount = std::size(lineVerts);
  lineVaoInfo.debugName = "test_fxaa_lines_vao";
  gfx::VertexArrayId lineVao = device.createVertexArray(lineVaoInfo);

  // Render lines to offscreen FBO
  device.bindFramebuffer(sceneFbo);
  device.setViewport(0, 0, w, h);
  device.setDepthTest(false);
  device.clearColor(std::nullopt, glm::vec4(0.02f, 0.02f, 0.05f, 1.0f));

  device.bindShaderProgram(lineShader);
  device.setLineWidth(2.0f);
  i32 modelLoc = device.getUniformLocation(lineShader, "modelMatrix");
  device.setUniformMat4(modelLoc, glm::mat4(1.0f));
  device.drawVertexArray(lineVao, lineVbo);
  glFinish();

  // Now apply FXAA pass: render fullscreen quad with scene texture
  gfx::ShaderId fxaaShader = resources.loadShaderProgram(
    "test_fxaa", "resources/Shaders/quad.vert", "resources/Shaders/Fxaa.frag");
  if (!device.isValid(fxaaShader)) {
    std::println(stderr, "  [fxaa] FXAA shader load failed");
    return false;
  }

  resources.bindShaderUniformBlock(
    fxaaShader, "PostProcessData", gfx::UBOBinding::PostProcess);

  // Set PostProcess UBO
  auto& pp = resources.getPostProcessUBO();
  pp.resolution =
    glm::vec4(static_cast<float>(w), static_cast<float>(h), 1.0f, 0.0f);
  pp.postConfig = glm::ivec4(0, 0, 0, 0);
  resources.flushPostProcessUBO();

  // Render FXAA to default framebuffer
  device.bindFramebuffer(device.getDefaultFramebuffer());
  device.setViewport(0, 0, w, h);
  device.setDepthTest(false);
  device.clearColor(std::nullopt, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

  device.bindShaderProgram(fxaaShader);

  // Bind scene texture to unit 0
  device.bindTexture(0, sceneTex);
  device.bindSampler(0, resources.getLinearClampSampler());
  i32 sceneLoc = device.getUniformLocation(fxaaShader, "scene");
  device.setUniformInt(sceneLoc, 0);

  resources.renderQuad();

  glFinish();
  return captureAndCompare(harness, "fxaa");
}

// ---------------------------------------------------------------------------
// Test 3: Bloom Combine — uses quad.vert + bloomCombine.frag
//
// Creates two textures (scene + bloom blur), then runs the engine's
// bloom combine shader which performs HDR tone-mapping and gamma correction.
// Tests the PostProcess UBO exposure control.
// ---------------------------------------------------------------------------

static bool
bloomCombineTest(VisualTestHarness& harness)
{
  auto& device = gfx::GraphicsDevice::getInstance();
  auto& resources = gfx::RenderResources::getInstance();

  const u32 w = static_cast<u32>(harness.getWidth());
  const u32 h = static_cast<u32>(harness.getHeight());

  // Create two source textures with procedural data:
  // - "scene": a gradient from dark to bright (HDR-ish values)
  // - "bloom": a soft glow in the center

  const u32 texW = 64;
  const u32 texH = 64;

  // Scene texture: horizontal gradient, values 0..2 (HDR range)
  std::vector<uint8_t> scenePixels(texW * texH * 4);
  for (u32 y = 0; y < texH; ++y) {
    for (u32 x = 0; x < texW; ++x) {
      u32 idx = (y * texW + x) * 4;
      float t = static_cast<float>(x) / static_cast<float>(texW - 1);
      // Store as RGBA8 (we'll use this with the tone-mapping shader)
      auto val = static_cast<uint8_t>(t * 255.0f);
      scenePixels[idx + 0] = val;
      scenePixels[idx + 1] = static_cast<uint8_t>(t * 200.0f);
      scenePixels[idx + 2] = static_cast<uint8_t>((1.0f - t) * 128.0f);
      scenePixels[idx + 3] = 255;
    }
  }

  // Bloom texture: radial falloff from center
  std::vector<uint8_t> bloomPixels(texW * texH * 4);
  for (u32 y = 0; y < texH; ++y) {
    for (u32 x = 0; x < texW; ++x) {
      u32 idx = (y * texW + x) * 4;
      float dx = (static_cast<float>(x) / static_cast<float>(texW - 1)) - 0.5f;
      float dy = (static_cast<float>(y) / static_cast<float>(texH - 1)) - 0.5f;
      float dist = std::sqrt(dx * dx + dy * dy);
      float glow = std::max(0.0f, 1.0f - dist * 3.0f);
      auto val = static_cast<uint8_t>(glow * 255.0f);
      bloomPixels[idx + 0] = val;
      bloomPixels[idx + 1] = static_cast<uint8_t>(glow * 200.0f);
      bloomPixels[idx + 2] = static_cast<uint8_t>(glow * 150.0f);
      bloomPixels[idx + 3] = 255;
    }
  }

  gfx::TextureCreateInfo texInfo{};
  texInfo.type = gfx::TextureType::Texture2D;
  texInfo.format = gfx::PixelFormat::RGBA8;
  texInfo.width = texW;
  texInfo.height = texH;
  texInfo.mipLevels = 1;
  texInfo.initialData = scenePixels.data();
  texInfo.debugName = "test_bloom_scene";
  gfx::TextureId sceneTexId =
    resources.createTexture2D("test_bloom_scene", texInfo);

  texInfo.initialData = bloomPixels.data();
  texInfo.debugName = "test_bloom_blur";
  gfx::TextureId bloomTexId =
    resources.createTexture2D("test_bloom_blur", texInfo);

  // Load bloom combine shader
  gfx::ShaderId bloomShader =
    resources.loadShaderProgram("test_bloomCombine",
                                "resources/Shaders/quad.vert",
                                "resources/Shaders/bloomCombine.frag");
  if (!device.isValid(bloomShader)) {
    std::println(stderr, "  [bloom_combine] Shader load failed");
    return false;
  }

  resources.bindShaderUniformBlock(
    bloomShader, "PostProcessData", gfx::UBOBinding::PostProcess);

  // Set PostProcess UBO with exposure
  auto& pp = resources.getPostProcessUBO();
  pp.resolution = glm::vec4(
    static_cast<float>(w), static_cast<float>(h), 1.5f, 0.0f); // z = exposure
  pp.postConfig = glm::ivec4(0, 0, 0, 0);
  resources.flushPostProcessUBO();

  // Render to default framebuffer
  device.bindFramebuffer(device.getDefaultFramebuffer());
  device.setViewport(0, 0, w, h);
  device.setDepthTest(false);
  device.clearColor(std::nullopt, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

  device.bindShaderProgram(bloomShader);

  // Bind scene texture to unit 0, bloom to unit 1
  device.bindTexture(0, sceneTexId);
  device.bindSampler(0, resources.getLinearClampSampler());
  i32 sceneLoc = device.getUniformLocation(bloomShader, "scene");
  device.setUniformInt(sceneLoc, 0);

  device.bindTexture(1, bloomTexId);
  device.bindSampler(1, resources.getLinearClampSampler());
  i32 bloomLoc = device.getUniformLocation(bloomShader, "bloomBlur");
  device.setUniformInt(bloomLoc, 1);

  resources.renderQuad();

  glFinish();
  return captureAndCompare(harness, "bloom_combine");
}

// ---------------------------------------------------------------------------
// Test 4: PBR Metal-Rough Spheres — full engine pipeline via FrameGraph + ECS
//
// Scene description: tests/scenes/pbr_metal_rough.yaml
// Loads the MetalRoughSpheres glTF model through the real engine pipeline:
// FrameGraph (Shadow→Geometry→Light→CubeMap→Bloom→FXAA) driven by ECS.
// This exercises the exact same code path as a live game scene.
// ---------------------------------------------------------------------------

static bool
pbrMetalRoughSpheresTest(VisualTestHarness& harness)
{
  return runSceneTest(harness, "pbr_metal_rough.yaml", "pbr_metal_rough");
}

// ---------------------------------------------------------------------------
// Test 5: Alpha Blend Mode — full engine pipeline via FrameGraph + ECS
//
// Scene description: tests/scenes/alpha_blend_mode.yaml
// Loads the AlphaBlendModeTest glTF model through the real engine pipeline.
// The model contains opaque, alpha-masked, and alpha-blended primitives,
// exercising the transparency path of the PBR material system.
// ---------------------------------------------------------------------------

static bool
alphaBlendModeTest(VisualTestHarness& harness)
{
  return runSceneTest(harness, "alpha_blend_mode.yaml", "alpha_blend_mode");
}

// ---------------------------------------------------------------------------
// Test 6: Normal Tangent Test — full engine pipeline via FrameGraph + ECS
//
// Scene descriptions:
//   tests/scenes/normal_tangent_front.yaml — camera straight-on
//   tests/scenes/normal_tangent_top.yaml   — camera above
//   tests/scenes/normal_tangent_back.yaml  — camera behind
//
// The model contains quads with varying normal/tangent configurations to
// verify that normal mapping is applied correctly in the PBR shader.
// ---------------------------------------------------------------------------

static bool
normalTangentTest(VisualTestHarness& harness)
{
  if (!runSceneTest(harness, "normal_tangent_front.yaml", "normal_tangent_front"))
    return false;
  if (!runSceneTest(harness, "normal_tangent_top.yaml", "normal_tangent_top"))
    return false;
  return runSceneTest(harness, "normal_tangent_back.yaml", "normal_tangent_back");
}

// Auto-register tests at static init
static bool reg1 = registerVisualTest("debug_lines", debugLinesTest);
static bool reg2 = registerVisualTest("fxaa", fxaaTest);
static bool reg3 = registerVisualTest("bloom_combine", bloomCombineTest);
static bool reg4 =
  registerVisualTest("pbr_metal_rough", pbrMetalRoughSpheresTest);
static bool reg5 = registerVisualTest("alpha_blend_mode", alphaBlendModeTest);
static bool reg6 = registerVisualTest("normal_tangent", normalTangentTest);
