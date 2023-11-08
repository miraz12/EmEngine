#ifndef PARTICLESCOMPONENT_H_
#define PARTICLESCOMPONENT_H_

#include "Component.hpp"

struct Particle {
  glm::vec3 position{0.0f}, velocity{0.0f};
  glm::vec4 color{1.0f};
  float life{0.0f};
  Particle(glm::vec3 pos, glm::vec3 vel)
      : position(pos), velocity(vel), color(1.0f), life(0.0f){};
};

class ParticlesComponent : public Component {
public:
  ParticlesComponent() = delete;
  ParticlesComponent(glm::vec3 position, glm::vec3 velocity)
      : m_position(position), m_velocity(velocity) {
    for (u32 i = 0; i < 50000; i++) {
      m_deadParticles.push_back(
          std::make_unique<Particle>(Particle(m_position, m_position)));
    }
  };
  virtual ~ParticlesComponent() = default;
  std::vector<std::unique_ptr<Particle>> &getAliveParticles() {
    return m_aliveParticles;
  }
  std::vector<std::unique_ptr<Particle>> &getDeadParticles() {
    return m_deadParticles;
  }
  glm::vec3 &getPosition() { return m_position; }
  glm::vec3 &getVelocity() { return m_velocity; }
  u32 getNumNewParticles() { return m_numNewParticles; }

private:
  std::vector<std::unique_ptr<Particle>> m_aliveParticles;
  std::vector<std::unique_ptr<Particle>> m_deadParticles;
  glm::vec3 m_position, m_velocity;
  u32 m_numNewParticles{20};
};

#endif // PARTICLESCOMPONENT_H_
