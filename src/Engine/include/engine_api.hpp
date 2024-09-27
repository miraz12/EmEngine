#ifndef ENGINE_API_HPP_
#define ENGINE_API_HPP_
#include "Core.hpp"
#include "ECS/ECSManager.hpp"

extern "C" void LoadScene(const char *filename) {
  ECSManager::getInstance().loadScene(filename);
}

extern "C" void ClearPressed();
extern "C" int GetPressed(int *vec);
extern "C" void Game_Update();
extern "C" bool Initialize() { return Core::getInstance().initialize(); }
extern "C" bool Open() { return Core::getInstance().open(); };
extern "C" void Update() { Core::getInstance().update(); };
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
