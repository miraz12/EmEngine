#ifndef GUI_H_
#define GUI_H_

#include <Game.hpp>

class GUI {

public:
  GUI() = delete;
  GUI(Game &game);
  ~GUI() = default;
  void renderGUI();
  void renderGizmos();

private:
  void editTransform(Camera &camera, glm::vec3 &pos, glm::vec3 &rot, glm::vec3 &scale);

  Game &m_game;
};

#endif // GUI_H_
