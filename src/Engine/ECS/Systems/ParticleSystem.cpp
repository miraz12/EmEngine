#include "ParticleSystem.hpp"
#include <ECS/ECSManager.hpp>
#include <algorithm>

void ParticleSystem::update(float dt) {
  std::vector<Entity> view = m_manager->view<ParticlesComponent>();
  for (auto &e : view) {
    std::shared_ptr<ParticlesComponent> partComp = m_manager->getComponent<ParticlesComponent>(e);
    for (uint32_t i = 0; i < partComp->getNumNewParticles(); i++) {
      reviveParticle(partComp);
    }
    std::vector<std::unique_ptr<Particle>> &aliveParticles = partComp->getAliveParticles();
    // Loop through all alive particles, removing all dying ones
    for (uint32_t i = 0; i < aliveParticles.size();)
      if (aliveParticles[i]->life <= 0) {
        killParticle(partComp, aliveParticles[i]);
      } else {
        aliveParticles[i]->velocity += glm::vec3(0.0, -9.8, 0.0) * dt;
        aliveParticles[i]->life -= dt;
        aliveParticles[i]->position += aliveParticles[i]->velocity * dt;
        aliveParticles[i]->color.a -= dt * 2.5f;
        i++; // Only progress if not deleting element as deleting shiftes the vector
      }
  }
}

void ParticleSystem::killParticle(std::shared_ptr<ParticlesComponent> pComp,
                                  std::unique_ptr<Particle> &p) {
  std::vector<std::unique_ptr<Particle>> &aliveParticles = pComp->getAliveParticles();
  std::vector<std::unique_ptr<Particle>> &deadParticles = pComp->getDeadParticles();
  std::vector<std::unique_ptr<Particle>>::iterator it =
      std::find(aliveParticles.begin(), aliveParticles.end(), p);
  if (it != aliveParticles.end()) {
    deadParticles.push_back(std::move(*it));
    aliveParticles.erase(it);
  }
}

void ParticleSystem::reviveParticle(std::shared_ptr<ParticlesComponent> pComp) {
  std::vector<std::unique_ptr<Particle>> &deadParticles = pComp->getDeadParticles();
  if (!deadParticles.empty()) {
    std::vector<std::unique_ptr<Particle>> &aliveParticles = pComp->getAliveParticles();
    std::unique_ptr<Particle> &p = deadParticles.back();
    // float random = ((rand() % 100) - 50) / 100.0f;
    float r = (distribution(generator) + 1.0) * 0.5;
    float g = (distribution(generator) + 1.0) * 0.5;
    float b = (distribution(generator) + 1.0) * 0.5;
    // std::cout << "Random: " << random << std::endl;
    // float rColor = 0.5f + ((rand() % 100) / 100.0f);
    glm::vec3 offset(0.0, 0.0, 0.0);
    p->position = pComp->getPosition() + offset;
    // Make particles glow with * 100
    p->color = glm::vec4(r * 100, g * 100, b * 100, 1.0f);
    p->life = 1.0f;
    p->velocity = pComp->getVelocity() + glm::vec3(distribution(generator), distribution(generator),
                                                   distribution(generator));
    aliveParticles.push_back(std::move(deadParticles.back()));
    deadParticles.pop_back();
  }
}
