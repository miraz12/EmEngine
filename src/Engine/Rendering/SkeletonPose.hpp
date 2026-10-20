#ifndef SKELETONPOSE_H_
#define SKELETONPOSE_H_

class GraphicsObject;

/// Per-entity animated pose for a GraphicsObject.
///
/// ResourceManager caches one GltfObject per filename, so every entity using
/// the same model shares a GraphicsObject. Geometry, materials and skin bind
/// data are read-only and safely shared; the animated node transforms are
/// not. Writing those into the shared object makes instances overwrite each
/// other, so they are held here instead, along with the caches derived from
/// them. Node topology stays on the shared object.
class SkeletonPose
{
public:
  /// Sizes the pose for obj and seeds it from the model's bind pose. Only
  /// re-seeds when the node count changes, so it is safe to call per frame.
  void bindTo(GraphicsObject& obj);

  glm::vec3& translation(u32 node) { return m_trans[node]; }
  glm::quat& rotation(u32 node) { return m_rot[node]; }
  glm::vec3& scale(u32 node) { return m_scale[node]; }

  glm::mat4 getLocalMat(const GraphicsObject& obj, i32 node) const;
  glm::mat4 getMatrix(const GraphicsObject& obj, i32 node);
  const std::vector<glm::mat4>& jointMatrices(GraphicsObject& obj, i32 skin);

  /// Drops the world-matrix and joint-matrix caches. Call after changing any
  /// node TRS, i.e. once per animation update.
  void invalidate();

  [[nodiscard]] bool isBound() const { return !m_trans.empty(); }
  [[nodiscard]] u32 nodeCount() const
  {
    return static_cast<u32>(m_trans.size());
  }

  /// Per-sampler keyframe cursor. The samplers live on the shared
  /// GraphicsObject, so the cursor has to be stored per entity or two
  /// entities playing one clip at different times fight over it.
  [[nodiscard]] size_t keyframeHint(size_t samplerIndex) const
  {
    return samplerIndex < m_keyframeHints.size() ? m_keyframeHints[samplerIndex]
                                                 : 0;
  }
  void setKeyframeHint(size_t samplerIndex, size_t value)
  {
    if (samplerIndex >= m_keyframeHints.size()) {
      m_keyframeHints.resize(samplerIndex + 1, 0);
    }
    m_keyframeHints[samplerIndex] = value;
  }

private:
  // Animated local TRS, one entry per node.
  std::vector<glm::vec3> m_trans;
  std::vector<glm::quat> m_rot;
  std::vector<glm::vec3> m_scale;

  std::vector<std::pair<bool, glm::mat4>> m_matrixCache;
  std::vector<size_t> m_keyframeHints;

  struct SkinCache
  {
    bool valid = false;
    std::vector<glm::mat4> jointMatrices;
  };
  std::vector<SkinCache> m_skinCache;
};

#endif // SKELETONPOSE_H_
