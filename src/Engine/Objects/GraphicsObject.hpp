#ifndef GRAPHICSOBJECT_H_
#define GRAPHICSOBJECT_H_

#include <Camera.hpp>
#include <Rendering/Animation.hpp>
#include <Rendering/Material.hpp>
#include <Rendering/Mesh.hpp>
#include <Rendering/Node.hpp>
#include <Rendering/Primitive.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>

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
  u32 p_numMats{0};
  std::unique_ptr<Material[]> p_materials;
  u32 p_numMeshes{0};
  std::unique_ptr<Mesh[]> p_meshes;
  u32 p_numAnimations{0};
  std::unique_ptr<Animation[]> p_animations;
  Material defaultMat;
};

#endif // GRAPHICSOBJECT_H_
