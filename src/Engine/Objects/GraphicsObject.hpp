#ifndef GRAPHICSOBJECT_H_
#define GRAPHICSOBJECT_H_

#include "Rendering/Skin.hpp"
#include <Graphics/Handle.hpp>
#include <Rendering/Animation.hpp>
#include <Rendering/Material.hpp>
#include <Rendering/Mesh.hpp>
#include <Rendering/Node.hpp>
#include <Rendering/Primitive.hpp>
#include <glm/gtx/string_cast.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

class GraphicsObject
{
public:
  GraphicsObject() = default;
  virtual ~GraphicsObject() = default;

  // Add a new node to the object
  void newNode(glm::mat4 model);

  // Compute the local transformation matrix of the given node
  glm::mat4 getLocalMat(i32 node);
  // Compute the world matrix of a node by recursively combining with parent
  // matrices
  glm::mat4 getMatrix(i32 node);
  // Reset the matrix cache (call when node transforms change)
  void resetMatrixCache();

  // Physics collision shape (Jolt)
  JPH::ShapeRefC p_collisionShape;

  u32 p_numNodes{ 0 };                       // Number of nodes
  std::unique_ptr<Node[]> p_nodes;           // Array of nodes
  u32 p_numMats{ 0 };                        // Number of materials
  std::unique_ptr<Material[]> p_materials;   // Array of materials
  u32 p_numMeshes{ 0 };                      // Number of meshes
  std::unique_ptr<Mesh[]> p_meshes;          // Array of meshes
  u32 p_numAnimations{ 0 };                  // Number of animations
  std::unique_ptr<Animation[]> p_animations; // Array of animations
  Material defaultMat;                       // Default material
  u32 p_numSkins{ 0 };                       // Number of skins
  std::unique_ptr<Skin[]> p_skins;           // Array of skins

private:
  // Cache for computed matrices
  mutable std::vector<std::pair<bool, glm::mat4>> m_matrixCache;
};

#endif // GRAPHICSOBJECT_H_
