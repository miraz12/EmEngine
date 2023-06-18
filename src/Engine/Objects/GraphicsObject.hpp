#ifndef GRAPHICSOBJECT_H_
#define GRAPHICSOBJECT_H_

#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <Rendering/Material.hpp>
#include <Rendering/Mesh.hpp>
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
  virtual ~GraphicsObject();

  virtual void draw(const ShaderProgram &sPrg);
  virtual void drawGeom(const ShaderProgram &sPrg);
  btCollisionShape *p_coll;

protected:
  glm::mat4 p_modelMatrix = glm::identity<glm::mat4>();
  std::vector<Node *> p_nodes;
  uint32_t p_numMats{0};
  std::unique_ptr<Material[]> p_materials;
  uint32_t p_numMeshes{0};
  std::unique_ptr<Mesh[]> p_meshes;
  Material defaultMat;
};

#endif // GRAPHICSOBJECT_H_
