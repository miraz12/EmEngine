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

namespace gfx {
class CommandBuffer;
}

class GraphicsObject
{
public:
  GraphicsObject() = default;
  virtual ~GraphicsObject() = default;

  // Add a new node to the object
  void newNode(glm::mat4 model);

  virtual void draw(gfx::ShaderId shader);

  virtual void drawGeom(gfx::ShaderId shader);

  /// Record draw commands (with materials) into CommandBuffer.
  /// @param modelMatrixLoc Uniform location for modelMatrix; set per-node as
  ///                       entityModel * getMatrix(node)
  /// @param isSkinnedLoc   Uniform location for is_skinned, or -1 to skip
  virtual void recordDraw(gfx::CommandBuffer& cmd,
                          gfx::SamplerId sampler,
                          const glm::mat4& entityModel,
                          i32 modelMatrixLoc,
                          i32 isSkinnedLoc = -1);

  /// Record geometry-only draw commands into CommandBuffer (for shadow pass).
  /// @param modelMatrixLoc Uniform location for modelMatrix; set per-node as
  ///                       entityModel * getMatrix(node)
  /// @param isSkinnedLoc   Uniform location for is_skinned, or -1 to skip
  virtual void recordDrawGeom(gfx::CommandBuffer& cmd,
                              const glm::mat4& entityModel,
                              i32 modelMatrixLoc,
                              i32 isSkinnedLoc = -1);

  void applySkinning(gfx::ShaderId shader, i32 node);

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

  // Skinning cache
  struct SkinningCache
  {
    bool valid = false;                   // Is cache valid?
    std::vector<glm::mat4> jointMatrices; // Cached joint matrices
  };
  mutable std::vector<SkinningCache> m_skinningCache;
};

#endif // GRAPHICSOBJECT_H_
