#include "DebugPass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include <ECS/Components/DebugComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <ECS/Systems/CameraSystem.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>

DebugPass::DebugPass()
    : RenderPass("resources/Shaders/vertex.glsl",
                 "resources/Shaders/fragment.glsl") {

  p_shaderProgram.setUniformBinding("modelMatrix");
  p_shaderProgram.setUniformBinding("viewMatrix");
  p_shaderProgram.setUniformBinding("projMatrix");
  p_shaderProgram.setUniformBinding("meshMatrix");

  p_shaderProgram.setAttribBinding("POSITION");
}

void DebugPass::Execute(ECSManager &eManager) {
  p_shaderProgram.use();

  auto cam = static_pointer_cast<CameraComponent>(
      ECSManager::getInstance().getCamera());

  CameraSystem::bindProjViewMatrix(
      cam, p_shaderProgram.getUniformLocation("projMatrix"),
      p_shaderProgram.getUniformLocation("viewMatrix"));

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glEnable(GL_DEPTH_TEST);

  glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1,
                     GL_FALSE, glm::value_ptr(glm::identity<glm::mat4>()));

  PhysicsSystem::getInstance().m_dDraw.renderAndFlush();
}
