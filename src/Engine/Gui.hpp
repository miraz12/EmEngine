#ifndef GUI_H_
#define GUI_H_

#include "Camera.hpp"

class GUI {

public:
  GUI() = default;
  ~GUI() = default;
  void renderGUI();

private:
  void editTransform(Camera &camera, glm::vec3 &pos, glm::vec3 &rot,
                     glm::vec3 &scale);
};

#endif // GUI_H_
