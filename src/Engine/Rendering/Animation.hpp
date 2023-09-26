#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "glm/fwd.hpp"
#include <limits>
#include <vector>
class Animation {
public:
  Animation() = default;
  ~Animation() = default;

  struct AnimationSampler {
    enum InterpolationType { LINEAR, STEP, CUBICSPLINE };
    InterpolationType interpolation;
    std::vector<float> inputs;
    std::vector<glm::vec4> outputsVec4;
  };

  struct AnimationChannel {
    enum PathType { TRANSLATION, ROTATION, SCALE };
    PathType path;
    // Node *node;
    uint32_t samplerIndex;
  };

  std::string name;
  std::vector<AnimationSampler> samplers;
  std::vector<AnimationChannel> channels;
  float start = std::numeric_limits<float>::max();
  float end = std::numeric_limits<float>::min();
};

#endif // ANIMATION_H_