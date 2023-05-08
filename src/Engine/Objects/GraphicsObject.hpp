#ifndef GRAPHICSOBJECT_H_
#define GRAPHICSOBJECT_H_

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "../ShaderPrograms/ShaderProgram.hpp"
#include "Camera.hpp"

class GraphicsObject {
public:
  GraphicsObject() = default;
  virtual ~GraphicsObject() = default;

  virtual void draw(ShaderProgram &sPrg) = 0;
  virtual void drawGeom(ShaderProgram &sPrg) = 0;

protected:
  glm::mat4 p_modelMatrix = glm::mat4(1.0f);
  std::vector<float> p_vertices;
  std::vector<float> p_normals;
  std::vector<float> p_texCoords;
  std::vector<int> p_indices;
  // std::vector<Material> p_materials;
};

#endif // GRAPHICSOBJECT_H_
