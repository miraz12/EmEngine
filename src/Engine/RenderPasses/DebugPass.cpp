#ifndef EMSCRIPTEN
#include "DebugPass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include <ECS/Components/DebugComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <ECS/Systems/CameraSystem.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>

DebugPass::DebugPass()
  : RenderPass("resources/Shaders/debugLine.vert",
               "resources/Shaders/debugLine.frag")
{
  // Set up for 3D debug rendering

  // Configure attribute bindings
  m_shaderProgram.setAttribBinding("POSITION"); // location 0
  m_shaderProgram.setAttribBinding("COLOR");    // location 1

  // Set up matrix uniforms
  m_shaderProgram.setUniformBinding("modelMatrix");
  m_shaderProgram.setUniformBinding("viewMatrix");
  m_shaderProgram.setUniformBinding("projMatrix");
}

void
DebugPass::Execute(ECSManager& eManager)
{
  // No startup logging needed

  // Debug rendering should go to the default framebuffer (screen)
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Save the current OpenGL state
  GLboolean depthMaskEnabled;
  glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMaskEnabled);
  GLint depthFunc;
  glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
  GLfloat lineWidth;
  glGetFloatv(GL_LINE_WIDTH, &lineWidth);

  // Set up OpenGL state for debug rendering
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glDepthMask(GL_FALSE); // Don't write to depth buffer

  // Make sure we can see lines clearly
  glEnable(GL_LINE_SMOOTH);
  glLineWidth(3.0f);

  // Enable blending for line transparency if needed
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Use the debug shader
  m_shaderProgram.use();

  // Get camera component
  auto camera = static_pointer_cast<CameraComponent>(eManager.getCamera());

  // Get matrices from the camera for correct 3D rendering
  glm::mat4 viewMatrix = camera->m_viewMatrix;
  glm::mat4 projMatrix = camera->m_ProjectionMatrix;

  // Now proceed with screen-space debug overlay

  // Disable depth testing for overlay
  glDisable(GL_DEPTH_TEST);

  // Now render the physics debug lines with proper 3D transformation
  // Pass our shader program to the DebugDrawer
  PhysicsSystem::getInstance().getDebugDrawer().renderAndFlush(
    viewMatrix, projMatrix, &m_shaderProgram);

  // Restore the previous OpenGL state
  glDepthMask(depthMaskEnabled);
  glDepthFunc(depthFunc);
  glLineWidth(lineWidth);
  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_BLEND);
}
#endif
