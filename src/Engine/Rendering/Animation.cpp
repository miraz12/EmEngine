#include "Animation.hpp"

// Find the appropriate keyframe index for the given time
// Uses the lastIndex as a hint to avoid searching from the beginning each time
size_t
AnimationSampler::findKeyframeIndex(float time) const
{
  // Check if we can use the cached index
  if (lastIndex < inputs.size() - 1) {
    if (time >= inputs[lastIndex] && time <= inputs[lastIndex + 1]) {
      return lastIndex;
    }

    // Try the next index
    if (lastIndex + 1 < inputs.size() - 1 && time >= inputs[lastIndex + 1] &&
        time <= inputs[lastIndex + 2]) {
      lastIndex++;
      return lastIndex;
    }
  }

  // Fall back to binary search
  size_t left = 0;
  size_t right = inputs.size() - 2;

  while (left <= right) {
    size_t mid = left + (right - left) / 2;

    if (time >= inputs[mid] && time <= inputs[mid + 1]) {
      lastIndex = mid;
      return mid;
    }

    if (inputs[mid] > time) {
      if (mid == 0)
        break;
      right = mid - 1;
    } else {
      left = mid + 1;
    }
  }

  // Default to the first keyframe if no suitable range is found
  lastIndex = 0;
  return 0;
}

// Cube spline interpolation function used for translate/scale/rotate with cubic
// spline animation samples Details on how this works can be found in the specs
// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
glm::vec4
AnimationSampler::cubicSplineInterpolation(size_t index,
                                           float time,
                                           uint32_t stride)
{
  float delta = inputs[index + 1] - inputs[index];
  float t = (time - inputs[index]) / delta;
  const size_t current = index * stride * 3;
  const size_t next = (index + 1) * stride * 3;
  const size_t A = 0;
  const size_t V = stride * 1;
  // const size_t B = stride * 2;

  float t2 = powf(t, 2);
  float t3 = powf(t, 3);
  glm::vec4 pt{ 0.0f };
  for (uint32_t i = 0; i < stride; i++) {
    float p0 = outputs[current + i + V]; // starting point at t = 0
    float m0 =
      delta * outputs[current + i + A]; // scaled starting tangent at t = 0
    float p1 = outputs[next + i + V];   // ending point at t = 1
    // float m1 = delta * outputs[next + i + B]; // scaled ending tangent at t
    // =1
    pt[i] = ((2.f * t3 - 3.f * t2 + 1.f) * p0) + ((t3 - 2.f * t2 + t) * m0) +
            ((-2.f * t3 + 3.f * t2) * p1) + ((t3 - t2) * m0);
  }
  return pt;
}

// Calculates the translation of this sampler for the given node at a given time
// point depending on the interpolation type
glm::vec3
AnimationSampler::translate(size_t index, float time)
{
  switch (interpolation) {
    case AnimationSampler::InterpolationType::LINEAR: {
      float delta = inputs[index + 1] - inputs[index];
      if (delta > 0.0f) {
        float u = std::max(0.0f, time - inputs[index]) / delta;
        return glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
      }
      return outputsVec4[index];
    }
    case AnimationSampler::InterpolationType::STEP: {
      return outputsVec4[index];
    }
    case AnimationSampler::InterpolationType::CUBICSPLINE: {
      return cubicSplineInterpolation(index, time, 3);
    }
    default:
      return outputsVec4[index];
  }
}

// Calculates the scale of this sampler for the given node at a given time point
// depending on the interpolation type
glm::vec3
AnimationSampler::scale(size_t index, float time)
{
  switch (interpolation) {
    case AnimationSampler::InterpolationType::LINEAR: {
      float delta = inputs[index + 1] - inputs[index];
      if (delta > 0.0f) {
        float u = std::max(0.0f, time - inputs[index]) / delta;
        return glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
      }
      return outputsVec4[index];
    }
    case AnimationSampler::InterpolationType::STEP: {
      return outputsVec4[index];
    }
    case AnimationSampler::InterpolationType::CUBICSPLINE: {
      return cubicSplineInterpolation(index, time, 3);
    }
    default:
      return outputsVec4[index];
  }
}

// Calculates the rotation of this sampler for the given node at a given time
// point depending on the interpolation type
glm::quat
AnimationSampler::rotate(size_t index, float time)
{
  switch (interpolation) {
    case AnimationSampler::InterpolationType::LINEAR: {
      float delta = inputs[index + 1] - inputs[index];
      if (delta > 0.0f) {
        float u = std::max(0.0f, time - inputs[index]) / delta;

        // Direct access to vec4 components is more efficient
        const glm::vec4& v1 = outputsVec4[index];
        const glm::vec4& v2 = outputsVec4[index + 1];

        glm::quat q1(v1.w, v1.x, v1.y, v1.z);
        glm::quat q2(v2.w, v2.x, v2.y, v2.z);

        return glm::normalize(glm::slerp(q1, q2, u));
      }

      const glm::vec4& v = outputsVec4[index];
      return glm::quat(v.w, v.x, v.y, v.z);
    }
    case AnimationSampler::InterpolationType::STEP: {
      const glm::vec4& v = outputsVec4[index];
      return glm::quat(v.w, v.x, v.y, v.z);
    }
    case AnimationSampler::InterpolationType::CUBICSPLINE: {
      glm::vec4 rot = cubicSplineInterpolation(index, time, 4);
      return glm::quat(rot.w, rot.x, rot.y, rot.z);
    }
    default: {
      const glm::vec4& v = outputsVec4[index];
      return glm::quat(v.w, v.x, v.y, v.z);
    }
  }
}
