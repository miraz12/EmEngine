#ifndef SCENELOADER_H_
#define SCENELOADER_H_
#include <Singleton.hpp>

class ECSManager;
class SceneLoader : public Singleton<SceneLoader> {
  friend class Singleton<SceneLoader>;

public:
  void init(std::string_view sceneFile);
  void saveScene(std::string_view sceneFile);

private:
  SceneLoader() = default;
  ~SceneLoader() = default;

  ECSManager *m_ecsMan;
};

#endif // SCENELOADER_H_
