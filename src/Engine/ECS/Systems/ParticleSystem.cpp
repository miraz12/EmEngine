#include "ParticleSystem.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include <ECS/ECSManager.hpp>

void
ParticleSystem::update(float dt)
{
  std::vector<Entity> view = m_manager->view<ParticlesComponent>();
  for (auto& e : view) {
    std::shared_ptr<ParticlesComponent> partComp =
      m_manager->getComponent<ParticlesComponent>(e);
    std::shared_ptr<PositionComponent> posComp =
      m_manager->getComponent<PositionComponent>(e);

    for (u32 i = 0; i < partComp->getNumNewParticles(); i++) {
      glm::vec3 pos = glm::vec3(0);
      if (posComp) {
        pos = posComp->position;
      }
      reviveParticle(partComp, pos);
    }
    std::vector<std::shared_ptr<Particle>>& aliveParticles =
      partComp->getAliveParticles();
    // Loop through all alive particles, removing all dying ones
    for (u32 i = 0; i < aliveParticles.size();)
      if (aliveParticles[i]->life <= 0) {
        killParticle(partComp, aliveParticles[i]);
      } else {
        aliveParticles[i]->velocity += glm::vec3(0.0, -9.8, 0.0) * dt;
        aliveParticles[i]->life -= dt;
        aliveParticles[i]->position += aliveParticles[i]->velocity * dt;
        aliveParticles[i]->color.a -= dt * 2.5f;
        i++; // Only progress if not deleting element as deleting shiftes the
             // vector
      }
  }
}

void
ParticleSystem::killParticle(std::shared_ptr<ParticlesComponent> pComp,
                             std::shared_ptr<Particle>& p)
{
  std::vector<std::shared_ptr<Particle>>& aliveParticles =
    pComp->getAliveParticles();
  std::vector<std::shared_ptr<Particle>>& deadParticles =
    pComp->getDeadParticles();
  std::vector<std::shared_ptr<Particle>>::iterator it =
    std::find(aliveParticles.begin(), aliveParticles.end(), p);
  if (it != aliveParticles.end()) {
    deadParticles.push_back(std::move(*it));
    aliveParticles.erase(it);
  }
}

void
ParticleSystem::reviveParticle(std::shared_ptr<ParticlesComponent> pComp,
                               glm::vec3& pos)
{
  std::vector<std::shared_ptr<Particle>>& deadParticles =
    pComp->getDeadParticles();
  if (!deadParticles.empty()) {
    std::vector<std::shared_ptr<Particle>>& aliveParticles =
      pComp->getAliveParticles();
    std::shared_ptr<Particle>& p = deadParticles.back();
    float r = (distribution(generator) + 1.0f) * 0.5f;
    float g = (distribution(generator) + 1.0f) * 0.5f;
    float b = (distribution(generator) + 1.0f) * 0.5f;
    glm::vec3 offset(0.0f, 0.0f, 0.0f);

    p->position = pos + offset;
    // Make particles glow
    float mul = 50.0f;
    p->color = glm::vec4(r * mul, g * mul, b * mul, 1.0f);
    p->life = 1.0f;
    p->velocity = pComp->getVelocity() + glm::vec3(distribution(generator),
                                                   distribution(generator),
                                                   distribution(generator));
    aliveParticles.push_back(std::move(deadParticles.back()));
    deadParticles.pop_back();
  }
}
