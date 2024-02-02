// #include <Game.hpp>
#include <Core.hpp>

static void mainLoop() { Core::getInstance().update(1); }

auto main(int /* argc */, char * /* argv */[]) -> int {
  if (Core::getInstance().initialize()) {
#ifdef EMSCRIPTEN
    emscripten_set_main_loop(&mainLoop, 0, 1);
#else
    while (Core::getInstance().open()) {
      mainLoop();
    }
#endif
  }
  return 0;
}
