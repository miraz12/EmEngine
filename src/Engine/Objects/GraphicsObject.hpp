#pragma once
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "../ShaderPrograms/ShaderProgram.hpp"

class GraphicsObject {
 public:
  GraphicsObject();
  virtual ~GraphicsObject();

  void bindVAO();
  void unbindVAO();
  virtual void draw(ShaderProgram &prog) = 0;

 protected:
  glm::mat4 p_modelMatrix = glm::mat4(1.0f);

  virtual void setVertexData(std::size_t dataSize, const void *data);
  virtual void setIndexData(std::size_t dataSize, const void *data);

 private:
  unsigned int m_VAO;
  unsigned int m_VBO;
  unsigned int m_EBO;  // Optional
};
