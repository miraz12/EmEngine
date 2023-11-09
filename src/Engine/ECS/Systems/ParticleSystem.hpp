#ifndef PARTICLESYSTEM_H_
#define PARTICLESYSTEM_H_

#include "System.hpp"
#include <ECS/Components/ParticlesComponent.hpp>

class ParticleSystem : public System, public Singleton<ParticleSystem> {
  friend class Singleton<ParticleSystem>;

public:
  void update(float dt) override;

private:
  ParticleSystem() = default;

  virtual ~ParticleSystem() = default;

  void killParticle(std::shared_ptr<ParticlesComponent> pComp,
                    std::unique_ptr<Particle> &p);
  void reviveParticle(std::shared_ptr<ParticlesComponent> pComp,
                      glm::vec3 &pos);

  std::default_random_engine generator;
  std::uniform_real_distribution<float> distribution{-1, 1};
};

#endif // PARTICLESYSTEM_H_
