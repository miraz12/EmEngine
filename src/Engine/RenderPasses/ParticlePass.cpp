#include "ParticlePass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "RenderUtil.hpp"
#include <ECS/Components/ParticlesComponent.hpp>

ParticlePass::ParticlePass()
  : RenderPass("resources/Shaders/particle.vert",
               "resources/Shaders/particle.frag")
{
  m_shaderProgram.setAttribBinding("POSITION");
  m_shaderProgram.setUniformBinding("viewMatrix");
  m_shaderProgram.setUniformBinding("projMatrix");
  m_shaderProgram.setUniformBinding("particlePos");
  m_shaderProgram.setUniformBinding("color");

  // Cache uniform locations for performance
  m_projMatrixLoc = m_shaderProgram.getUniformLocation("projMatrix");
  m_viewMatrixLoc = m_shaderProgram.getUniformLocation("viewMatrix");
  m_particlePosLoc = m_shaderProgram.getUniformLocation("particlePos");
  m_colorLoc = m_shaderProgram.getUniformLocation("color");
}

void
ParticlePass::Execute(ECSManager& eManager)
{
  m_shaderProgram.use();

  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  CameraSystem::bindProjViewMatrix(cam, m_projMatrixLoc, m_viewMatrixLoc);

  m_fboManager.bindFBO("cubeFBO");

  std::vector<Entity> view = eManager.view<ParticlesComponent>();
  for (auto& e : view) {
    std::shared_ptr<ParticlesComponent> pComp =
      eManager.getComponent<ParticlesComponent>(e);
    for (auto& p : pComp->getAliveParticles()) {
      if (p->life > 0.0f) {
        glUniform3fv(m_particlePosLoc, 1, glm::value_ptr(p->position));
        glUniform4fv(m_colorLoc, 1, glm::value_ptr(p->color));
        Util::renderQuad();
      }
    }
  }
}

void
ParticlePass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;
}
