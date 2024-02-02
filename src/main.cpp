// #include <Game.hpp>
#include <Core.hpp>
auto main(int /* argc */, char * /* argv */[]) -> int {
  // Game game;
  // game.run();
  Core core;
  if (core.initialize()) {
    while (core.open()) {
      core.update(1);
    }
  }
  return 0;
}
