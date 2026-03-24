#ifndef EMSCRIPTEN
#include "DebugPass.hpp"
#include <ECS/Systems/PhysicsSystem.hpp>
#include <Graphics/RenderResources.hpp>
#include <Graphics/UBOStructs.hpp>

DebugPass::DebugPass()
  : RenderPass("DebugPass",
               "resources/Shaders/debugLine.vert",
               "resources/Shaders/debugLine.frag")
{
  auto& resources = gfx::RenderResources::getInstance();

  // Bind CameraData uniform block
  useShader();
  resources.bindShaderUniformBlock(
    getShaderId(), "CameraData", gfx::UBOBinding::Camera);
}

void
DebugPass::Execute(ECSManager& /* eManager */)
{
  auto& resources = gfx::RenderResources::getInstance();

  // Debug rendering should go to the default framebuffer (screen)
  resources.bindDefaultFramebuffer();

  // Set up OpenGL state for debug rendering
  resources.setDepthTest(true);
  resources.setDepthFunc(gfx::CompareOp::LessEqual);
  resources.setDepthWrite(false);

  // Make sure we can see lines clearly
  resources.setLineSmoothing(true);
  resources.setLineWidth(1.0f);

  // Enable blending for line transparency if needed
  resources.setBlendEnabled(true);
  resources.setBlendFunc(gfx::BlendFactor::SrcAlpha,
                         gfx::BlendFactor::OneMinusSrcAlpha);

  // Use the debug shader
  useShader();

  // Disable depth testing for overlay
  resources.setDepthTest(false);

  // Render the physics debug lines (uses CameraData UBO for view/proj matrices)
  PhysicsSystem::getInstance().getDebugDrawer().renderAndFlush(getShaderId());

  // Restore the previous OpenGL state
  resources.setDepthWrite(true);
  resources.setDepthFunc(gfx::CompareOp::Less);
  resources.setLineWidth(1.0f);
  resources.setLineSmoothing(false);
  resources.setBlendEnabled(false);
}
#endif
