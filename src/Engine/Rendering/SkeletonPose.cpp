#include "SkeletonPose.hpp"

#include <Objects/GraphicsObject.hpp>

void
SkeletonPose::bindTo(GraphicsObject& obj)
{
  const u32 n = obj.p_numNodes;
  if (m_trans.size() == n) {
    return;
  }

  m_trans.resize(n);
  m_rot.resize(n);
  m_scale.resize(n);

  // Seed from the model's bind pose so an unanimated pose renders identically
  // to the shared object.
  for (u32 i = 0; i < n; i++) {
    m_trans[i] = obj.p_nodes[i].trans;
    m_rot[i] = obj.p_nodes[i].rot;
    m_scale[i] = obj.p_nodes[i].scale;
  }

  m_matrixCache.assign(n, { false, glm::mat4(1.0f) });
  m_skinCache.assign(obj.p_numSkins, {});
}

glm::mat4
SkeletonPose::getLocalMat(const GraphicsObject& obj, i32 node) const
{
  return glm::translate(glm::mat4(1.0f), m_trans[node]) *
         glm::toMat4(m_rot[node]) * glm::scale(glm::mat4(1.0f), m_scale[node]) *
         obj.p_nodes[node].nodeMat;
}

glm::mat4
SkeletonPose::getMatrix(const GraphicsObject& obj, i32 node)
{
  if (m_matrixCache[node].first) {
    return m_matrixCache[node].second;
  }

  const i32 parent = obj.p_nodes[node].parent;
  glm::mat4 result = (parent == -1)
                       ? getLocalMat(obj, node)
                       : getMatrix(obj, parent) * getLocalMat(obj, node);

  m_matrixCache[node] = { true, result };
  return result;
}

const std::vector<glm::mat4>&
SkeletonPose::jointMatrices(GraphicsObject& obj, i32 skin)
{
  if (m_skinCache.size() < static_cast<size_t>(obj.p_numSkins)) {
    m_skinCache.resize(obj.p_numSkins);
  }

  SkinCache& cache = m_skinCache[skin];
  if (cache.valid) {
    return cache.jointMatrices;
  }

  const Skin& s = obj.p_skins[skin];
  cache.jointMatrices.clear();
  cache.jointMatrices.reserve(s.joints.size());
  for (size_t j = 0; j < s.joints.size(); j++) {
    cache.jointMatrices.push_back(getMatrix(obj, s.joints[j]) *
                                  s.inverseBindMatrices[j]);
  }
  cache.valid = true;
  return cache.jointMatrices;
}

void
SkeletonPose::invalidate()
{
  for (auto& entry : m_matrixCache) {
    entry.first = false;
  }
  for (auto& cache : m_skinCache) {
    cache.valid = false;
  }
}
