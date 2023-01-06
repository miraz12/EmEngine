#pragma once

#include <ShaderPrograms/SimpleShaderProgram.hpp>
#include <glm/glm.hpp>

#include "GraphicsObject.hpp"

struct InstanceData;

class Quad : public GraphicsObject {
public:
  Quad(ShaderProgram &shaderProgram);
  virtual ~Quad();
  void draw(Camera& cam) override;

  glm::mat4 &getModelMatrix();
  unsigned int getQuadIndex();
  void setTextureIndex(unsigned int index);

private:
  float m_vertices[36] = {
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
      0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
  };

  unsigned int m_indices[6] = {
      0, 1, 2, 2, 1, 3,
  };

  glm::mat4 m_model;

private:
  void setVertexData(std::size_t dataSize, const void *data);
  void setIndexData(std::size_t dataSize, const void *data);
  unsigned int m_VBO;
  unsigned int m_EBO; // Optional
};
