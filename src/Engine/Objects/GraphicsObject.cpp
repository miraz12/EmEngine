#include "GraphicsObject.hpp"

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>

void
GraphicsObject::newNode(glm::mat4 model)
{
  p_numNodes = 1;
  p_nodes = std::make_unique<Node[]>(p_numNodes);
  p_nodes[0].mesh = 0;
  p_nodes[0].nodeMat = model;

  // Initialize matrix cache
  m_matrixCache.resize(p_numNodes, { false, glm::mat4(1.0f) });
}

glm::mat4
GraphicsObject::getLocalMat(i32 node)
{
  return glm::translate(glm::mat4(1.0f), p_nodes[node].trans) *
         glm::toMat4(p_nodes[node].rot) *
         glm::scale(glm::mat4(1.0f), p_nodes[node].scale) *
         p_nodes[node].nodeMat;
}

glm::mat4
GraphicsObject::getMatrix(i32 node)
{
  // Ensure the cache is properly sized
  if (m_matrixCache.size() < static_cast<size_t>(p_numNodes)) {
    m_matrixCache.resize(p_numNodes, { false, glm::mat4(1.0f) });
  }

  // Check if the matrix is already cached
  if (m_matrixCache[node].first) {
    return m_matrixCache[node].second;
  }

  // Calculate the matrix
  glm::mat4 result;
  if (p_nodes[node].parent == -1) {
    // If the node has no parent, use its local matrix
    result = getLocalMat(node);
  } else {
    // Recursively combine the parent transformation with the local matrix
    result = getMatrix(p_nodes[node].parent) * getLocalMat(node);
  }

  // Cache the result
  m_matrixCache[node] = { true, result };
  return result;
}

void
GraphicsObject::resetMatrixCache()
{
  // Mark all cached matrices as invalid
  for (auto& entry : m_matrixCache) {
    entry.first = false;
  }

  // Invalidate skinning cache when transforms change
  for (auto& cache : m_skinningCache) {
    cache.valid = false;
  }
}

void
GraphicsObject::applySkinning(gfx::ShaderId shader, i32 node)
{
  if (m_skinningCache.size() < static_cast<size_t>(p_numSkins)) {
    m_skinningCache.resize(p_numSkins);
  }

  SkinningCache& cache = m_skinningCache[node];

  // Recompute joint matrices only if cache is invalid
  if (!cache.valid) {
    cache.jointMatrices.clear();
    cache.jointMatrices.reserve(p_skins[node].joints.size());

    for (u32 j = 0; j < p_skins[node].joints.size(); j++) {
      i32 joint = p_skins[node].joints[j];
      cache.jointMatrices.push_back(getMatrix(joint) *
                                    p_skins[node].inverseBindMatrices[j]);
    }

    cache.valid = true;
  }

  // Upload joint matrices to GPU texture
  if (!cache.jointMatrices.empty()) {
    auto& resources = gfx::RenderResources::getInstance();
    auto& device = gfx::GraphicsDevice::getInstance();

    // Set jointMats sampler uniform to texture unit 5
    constexpr i32 kJointMatsUnit = 5;
    i32 jointMatsLoc = device.getUniformLocation(shader, "jointMats");
    device.setUniformInt(jointMatsLoc, kJointMatsUnit);
    resources.bindTexture(kJointMatsUnit, "jointMats");

    // Update texture data (width=4 for mat4 columns, height=jointCount)
    // Always reallocate since the jointMats texture is shared globally and
    // different skinned objects may have different joint counts.
    u32 jointCount = static_cast<u32>(cache.jointMatrices.size());
    constexpr u32 kMatrixColumns = 4;
    resources.updateDataTexture("jointMats",
                                kMatrixColumns,
                                jointCount,
                                glm::value_ptr(cache.jointMatrices[0]),
                                true); // Always reallocate
  }
}

void
GraphicsObject::draw(gfx::ShaderId shader)
{
  auto& device = gfx::GraphicsDevice::getInstance();

  for (u32 i = 0; i < p_numNodes; i++) {
    if (p_nodes[i].mesh >= 0) {
      i32 isSkinnedLoc = device.getUniformLocation(shader, "is_skinned");
      if (p_nodes[i].skin >= 0) {
        device.setUniformInt(isSkinnedLoc, 1);
        applySkinning(shader, p_nodes[i].skin);
      } else {
        device.setUniformInt(isSkinnedLoc, 0);
      }

      Mesh& mesh = p_meshes[p_nodes[i].mesh];
      for (u32 j = 0; j < mesh.numPrims; j++) {
        Material* mat = mesh.m_primitives[j].m_material > -1
                          ? &p_materials[mesh.m_primitives[j].m_material]
                          : &defaultMat;

        mat->bind(shader);
        mesh.m_primitives[j].draw();
      }
    }
  }
}

void
GraphicsObject::drawGeom(gfx::ShaderId shader)
{
  auto& device = gfx::GraphicsDevice::getInstance();

  for (u32 i = 0; i < p_numNodes; i++) {
    if (p_nodes[i].mesh >= 0) {
      i32 isSkinnedLoc = device.getUniformLocation(shader, "is_skinned");
      if (p_nodes[i].skin >= 0) {
        device.setUniformInt(isSkinnedLoc, 1);
        applySkinning(shader, p_nodes[i].skin);
      } else {
        device.setUniformInt(isSkinnedLoc, 0);
      }

      Mesh& mesh = p_meshes[p_nodes[i].mesh];
      for (u32 j = 0; j < mesh.numPrims; j++) {
        mesh.m_primitives[j].draw();
      }
    }
  }
}

void
GraphicsObject::recordDraw(gfx::CommandBuffer& cmd,
                           gfx::SamplerId sampler,
                           const glm::mat4& entityModel,
                           i32 modelMatrixLoc,
                           i32 isSkinnedLoc)
{
  auto& resources = gfx::RenderResources::getInstance();

  for (u32 i = 0; i < p_numNodes; i++) {
    if (p_nodes[i].mesh >= 0) {
      bool isSkinned = p_nodes[i].skin >= 0;

      // Skinned meshes: modelMatrix is the entity transform only (skinning
      // matrices already encode node-to-world via getMatrix(joint)).
      // Non-skinned meshes: bake the node's local-to-world into modelMatrix.
      if (modelMatrixLoc >= 0) {
        cmd.setUniform(modelMatrixLoc,
                       isSkinned ? entityModel : entityModel * getMatrix(i));
      }

      // Set is_skinned uniform if location is valid
      if (isSkinnedLoc >= 0) {
        cmd.setUniform(isSkinnedLoc, isSkinned ? 1 : 0);
      }

      // Handle skinning: upload joint matrices (immediate), then record
      // bindings
      if (isSkinned) {
        // Compute joint matrices if cache invalid
        if (m_skinningCache.size() < static_cast<size_t>(p_numSkins)) {
          m_skinningCache.resize(p_numSkins);
        }
        SkinningCache& cache = m_skinningCache[p_nodes[i].skin];
        if (!cache.valid) {
          cache.jointMatrices.clear();
          cache.jointMatrices.reserve(p_skins[p_nodes[i].skin].joints.size());
          for (u32 j = 0; j < p_skins[p_nodes[i].skin].joints.size(); j++) {
            i32 joint = p_skins[p_nodes[i].skin].joints[j];
            cache.jointMatrices.push_back(
              getMatrix(joint) *
              p_skins[p_nodes[i].skin].inverseBindMatrices[j]);
          }
          cache.valid = true;
        }

        // Upload joint matrices to texture (immediate operation)
        // Must bind texture to unit before updating!
        constexpr u32 kJointMatsUnit = 5;
        if (!cache.jointMatrices.empty()) {
          constexpr u32 kMatrixColumns = 4;
          u32 jointCount = static_cast<u32>(cache.jointMatrices.size());
          resources.bindTexture(kJointMatsUnit, "jointMats");
          resources.updateDataTexture("jointMats",
                                      kMatrixColumns,
                                      jointCount,
                                      glm::value_ptr(cache.jointMatrices[0]),
                                      true);
        }
      }

      Mesh& mesh = p_meshes[p_nodes[i].mesh];
      for (u32 j = 0; j < mesh.numPrims; j++) {
        // If skinned, record jointMats binding for EACH primitive
        // to ensure it's bound right before the draw
        if (isSkinned) {
          constexpr u32 kJointMatsUnit = 5;
          cmd.bindTextureByName(
            kJointMatsUnit, "jointMats", resources.getLinearClampSampler());
        }

        Material* mat = mesh.m_primitives[j].m_material > -1
                          ? &p_materials[mesh.m_primitives[j].m_material]
                          : &defaultMat;

        mat->recordBind(cmd, sampler);
        mesh.m_primitives[j].recordDraw(cmd);
      }
    }
  }
}

void
GraphicsObject::recordDrawGeom(gfx::CommandBuffer& cmd,
                               const glm::mat4& entityModel,
                               i32 modelMatrixLoc,
                               i32 isSkinnedLoc)
{
  auto& resources = gfx::RenderResources::getInstance();

  for (u32 i = 0; i < p_numNodes; i++) {
    if (p_nodes[i].mesh >= 0) {
      bool isSkinned = p_nodes[i].skin >= 0;

      // Skinned meshes: modelMatrix is the entity transform only (skinning
      // matrices already encode node-to-world via getMatrix(joint)).
      // Non-skinned meshes: bake the node's local-to-world into modelMatrix.
      if (modelMatrixLoc >= 0) {
        cmd.setUniform(modelMatrixLoc,
                       isSkinned ? entityModel : entityModel * getMatrix(i));
      }

      // Set is_skinned uniform if location is valid
      if (isSkinnedLoc >= 0) {
        cmd.setUniform(isSkinnedLoc, isSkinned ? 1 : 0);
      }

      // Handle skinning for shadow pass
      if (isSkinned) {
        // Compute joint matrices if cache invalid
        if (m_skinningCache.size() < static_cast<size_t>(p_numSkins)) {
          m_skinningCache.resize(p_numSkins);
        }
        SkinningCache& cache = m_skinningCache[p_nodes[i].skin];
        if (!cache.valid) {
          cache.jointMatrices.clear();
          cache.jointMatrices.reserve(p_skins[p_nodes[i].skin].joints.size());
          for (u32 j = 0; j < p_skins[p_nodes[i].skin].joints.size(); j++) {
            i32 joint = p_skins[p_nodes[i].skin].joints[j];
            cache.jointMatrices.push_back(
              getMatrix(joint) *
              p_skins[p_nodes[i].skin].inverseBindMatrices[j]);
          }
          cache.valid = true;
        }

        // Upload joint matrices to texture (immediate operation)
        // Must bind texture to unit before updating!
        constexpr u32 kJointMatsUnit = 5;
        if (!cache.jointMatrices.empty()) {
          constexpr u32 kMatrixColumns = 4;
          u32 jointCount = static_cast<u32>(cache.jointMatrices.size());
          resources.bindTexture(kJointMatsUnit, "jointMats");
          resources.updateDataTexture("jointMats",
                                      kMatrixColumns,
                                      jointCount,
                                      glm::value_ptr(cache.jointMatrices[0]),
                                      true);
        }

        // Record: bind jointMats texture to unit 5 (for command buffer
        // execution)
        cmd.bindTextureByName(
          kJointMatsUnit, "jointMats", resources.getLinearClampSampler());
      }

      Mesh& mesh = p_meshes[p_nodes[i].mesh];
      for (u32 j = 0; j < mesh.numPrims; j++) {
        mesh.m_primitives[j].recordDraw(cmd);
      }
    }
  }
}
