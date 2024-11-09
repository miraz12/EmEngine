#ifndef ENGINE_API_HPP_
#define ENGINE_API_HPP_
#include "Core.hpp"
#include "ECS/ECSManager.hpp"

extern "C" void LoadScene(const char *filename) {
  ECSManager::getInstance().loadScene(filename);
}

extern "C" void AddPositionComponent(int entity, float pos[3], float scale[3],
                                     float rot[3]);
extern "C" void AddPhysicsComponent(int entity, float mass, int type);
extern "C" void AddGraphicsComponent(int entity, const char *model);
extern "C" int CreateEntity(const char *name);
extern "C" int GetPressed(int *vec);
extern "C" void Game_Update();
extern "C" bool Initialize() { return Core::getInstance().initialize(); }
extern "C" bool Open() { return Core::getInstance().open(); };
extern "C" void Update() { Core::getInstance().update(); };
extern "C" void SetVelocity(unsigned int entity, float x, float y, float z);
extern "C" void SetAcceleration(unsigned int entity, float x, float y, float z);
extern "C" void Start() {

#ifdef EMSCRIPTEN
  emscripten_set_main_loop(&Game_Update, 0, 1);
#else
  while (Core::getInstance().open()) {
    Game_Update();
  }
#endif
}

#endif // ENGINE_API_HPP_
