#ifndef SCENELOADER_H_
#define SCENELOADER_H_
#include <ECS/ECSManager.hpp>
#include <Singleton.hpp>

class SceneLoader : public Singleton<SceneLoader> {
  friend class Singleton<SceneLoader>;

public:
  void init(std::string sceneFile);
  void saveScene(std::string sceneFile);

private:
  SceneLoader() = default;
  ~SceneLoader() = default;

  ECSManager *m_ecsMan;
};

#endif // SCENELOADER_H_
