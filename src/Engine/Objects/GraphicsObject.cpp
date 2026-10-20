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
}
