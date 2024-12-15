#ifndef GRAPHICSOBJECT_H_
#define GRAPHICSOBJECT_H_

#include "Rendering/Skin.hpp"
#include <Rendering/Animation.hpp>
#include <Rendering/Material.hpp>
#include <Rendering/Mesh.hpp>
#include <Rendering/Node.hpp>
#include <Rendering/Primitive.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>
#include <glm/gtx/string_cast.hpp>

class GraphicsObject {
public:
  GraphicsObject() = default;
  virtual ~GraphicsObject() = default;

  // Add a new node to the object
  void newNode(glm::mat4 model);

  virtual void draw(const ShaderProgram &sPrg);

  virtual void drawGeom(const ShaderProgram &sPrg);

  void applySkinning(const ShaderProgram &sPrg, i32 node);

  // Compute the local transformation matrix of the given node
  glm::mat4 getLocalMat(i32 node);
  // Compute the world matrix of a node by recursively combining with parent
  // matrices
  glm::mat4 getMatrix(i32 node);

  // Bullet Physics collision shape
  btCollisionShape *p_coll;

  u32 p_numNodes{0};                         // Number of nodes
  std::unique_ptr<Node[]> p_nodes;           // Array of nodes
  u32 p_numMats{0};                          // Number of materials
  std::unique_ptr<Material[]> p_materials;   // Array of materials
  u32 p_numMeshes{0};                        // Number of meshes
  std::unique_ptr<Mesh[]> p_meshes;          // Array of meshes
  u32 p_numAnimations{0};                    // Number of animations
  std::unique_ptr<Animation[]> p_animations; // Array of animations
  Material defaultMat;                       // Default material
  u32 p_numSkins{0};                         // Number of skins
  std::unique_ptr<Skin[]> p_skins;           // Array of skins
};

#endif // GRAPHICSOBJECT_H_
