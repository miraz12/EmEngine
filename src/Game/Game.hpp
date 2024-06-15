#ifndef GAME_H_
#define GAME_H_

#include "engine_api.hpp"
#include "include/game_api.hpp"

class Game {
public:
  Game();
  ~Game() = default;

  void update(float dt);
};

#endif // GAME_H_
