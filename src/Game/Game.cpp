#include "Game.hpp"

static Game *game;
Game::Game() { ECS_loadScene("resources/scene.yaml"); }

void Game::update(float dt) {}

extern "C" void game_initialize() { game = new Game(); }
extern "C" void game_update(float dt) { game->update(dt); }
