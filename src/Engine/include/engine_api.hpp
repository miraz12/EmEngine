#ifndef ENGINE_API_HPP_
#define ENGINE_API_HPP_
#include "Core.hpp"
#include "ECS/ECSManager.hpp"

extern "C" {

void LoadScene(const char *filename) {
  ECSManager::getInstance().loadScene(filename);
}

void AddPositionComponent(int entity, float pos[3], float scale[3],
                          float rot[3]);
void AddPhysicsComponent(int entity, float mass, int type);
void AddGraphicsComponent(int entity, const char *model);
int CreateEntity(const char *name);
int GetPressed(int *vec);
void Game_Update();
bool Initialize() { return Core::getInstance().initialize(); }
bool Open() { return Core::getInstance().open(); };
void Update() { Core::getInstance().update(); };
void SetVelocity(unsigned int entity, float x, float y, float z);
void AddImpulse(unsigned int entity, float x, float y, float z);
void AddForce(unsigned int entity, float x, float y, float z);
void SetAcceleration(unsigned int entity, float x, float y, float z);
void Start() {

#ifdef EMSCRIPTEN
  emscripten_set_main_loop(&Game_Update, 0, 1);
#else
  while (Core::getInstance().open()) {
    Game_Update();
  }
#endif
}
};

#endif // ENGINE_API_HPP_
