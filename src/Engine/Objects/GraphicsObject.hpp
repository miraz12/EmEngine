#ifndef GRAPHICSOBJECT_H_
#define GRAPHICSOBJECT_H_

#include <Rendering/Material.hpp>
#include <Rendering/MeshObj.hpp>
#include <Rendering/Node.hpp>
#include <Rendering/Primitive.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <map>
#include <vector>

#include "../ShaderPrograms/ShaderProgram.hpp"
#include "Camera.hpp"

class GraphicsObject {
public:
  GraphicsObject() = default;
  virtual ~GraphicsObject() = default;

  virtual void draw(const ShaderProgram &sPrg) = 0;
  virtual void drawGeom(const ShaderProgram &sPrg) = 0;

protected:
  glm::mat4 p_modelMatrix = glm::identity<glm::mat4>();
  std::vector<Material *> p_materials;
  std::vector<MeshObj *> p_meshes;
  std::vector<Node *> p_nodes;
  Material defaultMat;
};

#endif // GRAPHICSOBJECT_H_
