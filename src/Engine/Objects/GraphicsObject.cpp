#include "GraphicsObject.hpp"

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
GraphicsObject::applySkinning(const ShaderProgram& sPrg, i32 node)
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
    glUniform1i(sPrg.getUniformLocation("jointMats"), 5);
    p_textureManager.bindActivateTexture("jointMats", 5);

    // Create texture on first use, then reuse with glTexSubImage2D
    if (cache.textureId == 0) {
      // First time: allocate texture storage
      glTexImage2D(GL_TEXTURE_2D,
                   0,                          // mipmap level
                   GL_RGBA32F,                 // internal format
                   4,                          // width (4 columns per matrix)
                   cache.jointMatrices.size(), // height (number of matrices)
                   0,                          // border
                   GL_RGBA,                    // format
                   GL_FLOAT,                   // type
                   glm::value_ptr(cache.jointMatrices[0])); // data
      cache.textureId = 1; // Mark as initialized
    } else {
      // Subsequent frames: update existing texture (faster)
      glTexSubImage2D(GL_TEXTURE_2D,
                      0,                                       // mipmap level
                      0,                                       // x offset
                      0,                                       // y offset
                      4,                                       // width
                      cache.jointMatrices.size(),              // height
                      GL_RGBA,                                 // format
                      GL_FLOAT,                                // type
                      glm::value_ptr(cache.jointMatrices[0])); // data
    }
  }
}

void
GraphicsObject::draw(const ShaderProgram& sPrg)
{

  for (u32 i = 0; i < p_numNodes; i++) {
    if (p_nodes[i].mesh >= 0) {
      if (p_nodes[i].skin >= 0) {
        glUniform1i(sPrg.getUniformLocation("is_skinned"), 1);
        applySkinning(sPrg, p_nodes[i].skin);
      } else {
        glUniform1i(sPrg.getUniformLocation("is_skinned"), 0);
      }

      Mesh& m = p_meshes[p_nodes[i].mesh];
      for (u32 j = 0; j < m.numPrims; j++) {
        Material* mat = m.m_primitives[j].m_material > -1
                          ? &p_materials[m.m_primitives[j].m_material]
                          : &defaultMat;

        mat->bind(sPrg);
        m.m_primitives[j].draw();
      }
    }
  }
}
void
GraphicsObject::drawGeom(const ShaderProgram&)
{
  for (u32 i = 0; i < p_numNodes; i++) {
    if (p_nodes[i].mesh >= 0) {
      Mesh& m = p_meshes[p_nodes[i].mesh];
      for (u32 i = 0; i < m.numPrims; i++) {
        m.m_primitives[i].draw();
      }
    }
  }
}
