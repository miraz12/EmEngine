#ifndef GUI_H_
#define GUI_H_

#include "Camera.hpp"
#include "ECS/Components/CameraComponent.hpp"

class GUI {

public:
  GUI() = default;
  ~GUI() = default;
  void renderGUI();

private:
  void editTransform(std::shared_ptr<CameraComponent> camera, glm::vec3 &pos,
                     glm::vec3 &rot, glm::vec3 &scale);
};

#endif // GUI_H_
