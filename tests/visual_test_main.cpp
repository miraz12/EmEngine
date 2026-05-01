#include "visual_test_harness.h"
#include "visual_test_utils.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <ECS/ECSManager.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <RenderPasses/FrameGraph.hpp>

#include <cstdlib>
#include <filesystem>
#include <functional>
#include <print>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Simple visual test registry
// ---------------------------------------------------------------------------

struct VisualTest
{
  std::string name;
  std::function<bool(VisualTestHarness&)> func;
};

static std::vector<VisualTest>&
getTestRegistry()
{
  static std::vector<VisualTest> registry;
  return registry;
}

bool
registerVisualTest(const std::string& name,
                   std::function<bool(VisualTestHarness&)> func)
{
  getTestRegistry().push_back({ name, std::move(func) });
  return true;
}

// Stub for engine_api.hpp's Game_Update (normally provided by the C# game)
extern "C" void
Game_Update()
{}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int
main()
{
  if (!glfwInit()) {
    std::println(stderr, "[main] Failed to initialize GLFW");
    return 1;
  }

  // Request an OpenGL 4.6 compatibility context (matches engine GLAD config)
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  constexpr int kWidth = 1600;
  constexpr int kHeight = 1200;

  VisualTestHarness harness;
  if (!harness.init(kWidth, kHeight)) {
    glfwTerminate();
    return 1;
  }

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::println(stderr, "[main] Failed to initialize GLAD");
    harness.shutdown();
    glfwTerminate();
    return 1;
  }

  // Initialize the engine's GraphicsDevice (required for gfx:: API calls)
  if (!gfx::GraphicsDevice::getInstance().initialize()) {
    std::println(stderr, "[main] Failed to initialize GraphicsDevice");
    harness.shutdown();
    glfwTerminate();
    return 1;
  }

  // Initialize RenderResources (shared UBOs, samplers, quad/cube geometry)
  if (!gfx::RenderResources::getInstance().initialize()) {
    std::println(stderr, "[main] Failed to initialize RenderResources");
    harness.shutdown();
    glfwTerminate();
    return 1;
  }

  // Initialize ECS systems (required before FrameGraph construction)
  ECSManager::getInstance().initializeSystems();

  // Set up a default GL readback callback
  harness.setCaptureCallback([](int w, int h, void* buf) {
    glFinish();
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buf);
  });

  // Run all registered tests
  auto& tests = getTestRegistry();
  int passed = 0;
  int failed = 0;

  std::println("Running {} visual test(s)...\n", tests.size());

  for (auto& test : tests) {
    std::println("  [RUN ] {}", test.name);
    bool ok = test.func(harness);
    if (ok) {
      std::println("  [PASS] {}", test.name);
      ++passed;
    } else {
      std::println("  [FAIL] {}", test.name);
      ++failed;
    }
  }

  std::println("\n--- Summary ---");
  std::println("  Passed: {}", passed);
  std::println("  Failed: {}", failed);

  harness.shutdown();
  glfwTerminate();

  return (failed > 0) ? 1 : 0;
}
