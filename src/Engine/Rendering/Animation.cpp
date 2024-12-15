#include "Animation.hpp"

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
      float u = std::max(0.0f, time - inputs[index]) /
                (inputs[index + 1] - inputs[index]);
      return glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
    }
    case AnimationSampler::InterpolationType::STEP: {
      return outputsVec4[index];
    }
    case AnimationSampler::InterpolationType::CUBICSPLINE: {
      return cubicSplineInterpolation(index, time, 3);
    }
  }
}

// Calculates the scale of this sampler for the given node at a given time point
// depending on the interpolation type
glm::vec3
AnimationSampler::scale(size_t index, float time)
{
  switch (interpolation) {
    case AnimationSampler::InterpolationType::LINEAR: {
      float u = std::max(0.0f, time - inputs[index]) /
                (inputs[index + 1] - inputs[index]);
      return glm::mix(outputsVec4[index], outputsVec4[index + 1], u);
    }
    case AnimationSampler::InterpolationType::STEP: {
      return outputsVec4[index];
    }
    case AnimationSampler::InterpolationType::CUBICSPLINE: {
      return cubicSplineInterpolation(index, time, 3);
    }
  }
}

// Calculates the rotation of this sampler for the given node at a given time
// point depending on the interpolation type
glm::quat
AnimationSampler::rotate(size_t index, float time)
{
  switch (interpolation) {
    case AnimationSampler::InterpolationType::LINEAR: {
      float u = std::max(0.0f, time - inputs[index]) /
                (inputs[index + 1] - inputs[index]);
      glm::quat q1;
      q1.x = outputsVec4[index].x;
      q1.y = outputsVec4[index].y;
      q1.z = outputsVec4[index].z;
      q1.w = outputsVec4[index].w;
      glm::quat q2;
      q2.x = outputsVec4[index + 1].x;
      q2.y = outputsVec4[index + 1].y;
      q2.z = outputsVec4[index + 1].z;
      q2.w = outputsVec4[index + 1].w;
      return glm::normalize(glm::slerp(q1, q2, u));
    }
    case AnimationSampler::InterpolationType::STEP: {
      glm::quat q1;
      q1.x = outputsVec4[index].x;
      q1.y = outputsVec4[index].y;
      q1.z = outputsVec4[index].z;
      q1.w = outputsVec4[index].w;
      return q1;
    }
    case AnimationSampler::InterpolationType::CUBICSPLINE: {
      glm::vec4 rot = cubicSplineInterpolation(index, time, 4);
      glm::quat q;
      q.x = rot.x;
      q.y = rot.y;
      q.z = rot.z;
      q.w = rot.w;
      return q;
    }
  }
}
