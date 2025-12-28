#ifndef SCENELOADER_H_
#define SCENELOADER_H_
#include <Singleton.hpp>

class ECSManager;
class SceneLoader : public Singleton<SceneLoader>
{
  friend class Singleton<SceneLoader>;

public:
  void init(const char* file);
  void saveScene(const char* file);

private:
  SceneLoader() = default;
  ~SceneLoader() = default;

  void addGraphicsComponent(Entity entity, const YAML::Node& component);
  void addPhysicsComponent(Entity entity, const YAML::Node& component);
  void addPositionComponent(Entity entity, const YAML::Node& component);
  void addParticlesComponent(Entity entity, const YAML::Node& component);
  void addDirectionalLight(Entity entity, const YAML::Node& component);
};

#endif // SCENELOADER_H_
