#ifndef SCENELOADER_H_
#define SCENELOADER_H_
#include <Singleton.hpp>

class ECSManager;
class SceneLoader : public Singleton<SceneLoader> {
  friend class Singleton<SceneLoader>;

public:
  void init(const char *file);
  void saveScene(const char *file);

private:
  SceneLoader() = default;
  ~SceneLoader() = default;

  ECSManager *m_ecsMan;
};

#endif // SCENELOADER_H_
