#include "ParticlePass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "RenderUtil.hpp"
#include <ECS/Components/ParticlesComponent.hpp>

ParticlePass::ParticlePass()
  : RenderPass("resources/Shaders/particle.vert",
               "resources/Shaders/particle.frag")
{
  p_shaderProgram.setAttribBinding("POSITION");
  p_shaderProgram.setUniformBinding("viewMatrix");
  p_shaderProgram.setUniformBinding("projMatrix");
  p_shaderProgram.setUniformBinding("particlePos");
  p_shaderProgram.setUniformBinding("color");
}

void
ParticlePass::Execute(ECSManager& eManager)
{
  p_shaderProgram.use();

  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  CameraSystem::bindProjViewMatrix(
    cam,
    p_shaderProgram.getUniformLocation("projMatrix"),
    p_shaderProgram.getUniformLocation("viewMatrix"));

  p_fboManager.bindFBO("cubeFBO");

  std::vector<Entity> view = eManager.view<ParticlesComponent>();
  for (auto& e : view) {
    std::shared_ptr<ParticlesComponent> pComp =
      eManager.getComponent<ParticlesComponent>(e);
    for (auto& p : pComp->getAliveParticles()) {
      if (p->life > 0.0f) {
        glUniform3fv(p_shaderProgram.getUniformLocation("particlePos"),
                     1,
                     glm::value_ptr(p->position));
        glUniform4fv(p_shaderProgram.getUniformLocation("color"),
                     1,
                     glm::value_ptr(p->color));
        Util::renderQuad();
      }
    }
  }
}

void
ParticlePass::setViewport(u32 w, u32 h)
{
  p_width = w;
  p_height = h;
}
