#pragma once
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "../ShaderPrograms/ShaderProgram.hpp"
#include "Camera.hpp"

class GraphicsObject {
public:
  GraphicsObject(ShaderProgram &shaderProgram);
  virtual ~GraphicsObject();

  virtual void draw(Camera &cam) = 0;

protected:
  glm::mat4 p_modelMatrix = glm::mat4(1.0f);
  ShaderProgram &p_shaderProgram;
};
