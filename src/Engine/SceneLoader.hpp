#ifndef SCENELOADER_H_
#define SCENELOADER_H_
#include <Singleton.hpp>
#include <string>

class SceneLoader : public Singleton<SceneLoader> {
  friend class Singleton<SceneLoader>;

public:
  void init(std::string sceneFile);

private:
  SceneLoader() = default;
  ~SceneLoader() = default;
};

#endif // SCENELOADER_H_
