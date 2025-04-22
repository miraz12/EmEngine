#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <Rendering/Node.hpp>

struct AnimationSampler
{
  enum class InterpolationType
  {
    LINEAR,
    STEP,
    CUBICSPLINE
  };
  InterpolationType interpolation;
  std::vector<float> inputs;
  std::vector<glm::vec4> outputsVec4;
  std::vector<float> outputs;

  // Pre-calculated time indices to avoid binary search during animation
  mutable size_t lastIndex = 0;

  // Find the appropriate keyframe index for the given time
  size_t findKeyframeIndex(float time) const;

  glm::vec3 translate(size_t index, float time);
  glm::vec3 scale(size_t index, float time);
  glm::quat rotate(size_t index, float time);
  glm::vec4 cubicSplineInterpolation(size_t index, float time, uint32_t stride);
};

struct AnimationChannel
{
  enum class PathType
  {
    TRANSLATION,
    ROTATION,
    SCALE
  };
  PathType path;
  u32 node;
  uint32_t samplerIndex;
};

class Animation
{
public:
  Animation() = default;
  ~Animation() = default;

  std::string name;
  std::vector<AnimationSampler> samplers;
  std::vector<AnimationChannel> channels;
  float start = std::numeric_limits<float>::max();
  float end = std::numeric_limits<float>::min();
};

#endif // ANIMATION_H_
