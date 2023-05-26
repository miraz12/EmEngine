#ifndef QUAD_H_
#define QUAD_H_

#include <ShaderPrograms/SimpleShaderProgram.hpp>
#include <glm/glm.hpp>

#include "GraphicsObject.hpp"

struct InstanceData;

class Quad : public GraphicsObject {
public:
  Quad(ShaderProgram &shaderProgram);
  virtual ~Quad();
  void draw(const ShaderProgram &sPrg) override;
  void drawGeom(const ShaderProgram &sPrg) override;

  glm::mat4 &getModelMatrix();
  uint32_t getQuadIndex();
  void setTextureIndex(uint32_t index);

private:
  float m_vertices[36] = {
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f,  0.0f, 1.0f, 0.5f, -0.5f, 0.0f,
      0.0f,  1.0f,  0.0f, 0.5f, 1.0f, 1.0f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f,  1.0f,
      0.5f,  0.0f,  0.0f, 0.5f, 0.5f, 0.0f, 1.0f,  1.0f, 0.0f, 0.5f, 1.0f,  0.0f,
  };

  uint32_t m_indices[6] = {
      0, 1, 2, 2, 1, 3,
  };

  void setVertexData(std::size_t dataSize, const void *data);
  void setIndexData(std::size_t dataSize, const void *data);
  uint32_t m_VBO;
  uint32_t m_EBO; // Optional

  uint32_t m_VAO;
};
#endif // QUAD_H_
