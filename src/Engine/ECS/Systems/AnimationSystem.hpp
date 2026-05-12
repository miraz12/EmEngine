#ifndef ANIMATIONSYSTEM_H_
#define ANIMATIONSYSTEM_H_

#include "System.hpp"
#include <Singleton.hpp>

class GraphicsObject;
class Animation;

class AnimationSystem final
  : public System
  , public Singleton<AnimationSystem>
{
  friend class Singleton<AnimationSystem>;

public:
  void update(float dt) override;

private:
  AnimationSystem() = default;
  ~AnimationSystem() override = default;

  void sampleAnimation(GraphicsObject* obj, Animation& animation, float time);
  // Snapshot current animation into snapshot storage. Used for blending
  void snapshotPose(GraphicsObject* obj);
  void blendPose(GraphicsObject* obj, float weight);

  // Reusable per-frame storage for source pose during crossfade
  std::vector<glm::vec3> m_snapTrans;
  std::vector<glm::quat> m_snapRot;
  std::vector<glm::vec3> m_snapScale;
};
#endif // ANIMATIONSYSTEM_H_
