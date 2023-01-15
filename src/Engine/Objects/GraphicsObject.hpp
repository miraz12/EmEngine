#ifndef GRAPHICSOBJECT_H_
#define GRAPHICSOBJECT_H_

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "../ShaderPrograms/ShaderProgram.hpp"
#include "Camera.hpp"

class GraphicsObject {
 public:
  GraphicsObject(ShaderProgram &shaderProgram);
  virtual ~GraphicsObject();

  virtual void draw(Camera &cam, glm::mat4 model) = 0;

 protected:
  glm::mat4 p_modelMatrix = glm::mat4(1.0f);
  ShaderProgram &p_shaderProgram;
};

#endif  // GRAPHICSOBJECT_H_
