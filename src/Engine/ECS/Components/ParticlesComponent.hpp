#ifndef PARTICLESCOMPONENT_H_
#define PARTICLESCOMPONENT_H_

struct Particle
{
  glm::vec3 position{ 0.0f };
  glm::vec3 velocity{ 0.0f };
  glm::vec4 color{ 1.0f };
  float life{ 0.0f };
  Particle(glm::vec3 pos, glm::vec3 vel)
    : position(pos)
    , velocity(vel) {};
};

struct ParticlesComponent
{
  ParticlesComponent() = delete;
  explicit ParticlesComponent(glm::vec3 vel)
    : velocity(vel)
  {
    for (u32 i = 0; i < 50000; i++) {
      deadParticles.push_back(
        std::make_unique<Particle>(Particle(glm::vec3(0), velocity)));
    }
  };
  ~ParticlesComponent() = default;

  std::vector<std::shared_ptr<Particle>> aliveParticles;
  std::vector<std::shared_ptr<Particle>> deadParticles;
  glm::vec3 velocity;
  u32 numNewParticles{ 10 };
};

#endif // PARTICLESCOMPONENT_H_
