#version 300 es
// =============================================================================
// Shader: bloomCombine.frag
// Purpose: Blend bloom with scene, apply tonemapping and gamma correction
// =============================================================================
precision highp float;

out vec4 FragColor;

in vec2 texCoords;

// Samplers remain as regular uniforms (cannot be in UBO)
uniform sampler2D scene;
uniform sampler2D bloomBlur;

// PostProcess UBO (binding point 5)
layout(std140) uniform PostProcessData
{
  vec4 resolution;      // xy = resolution, z = exposure, w = filterRadius
  ivec4 postConfig;     // x = mipLevel, y = sceneSampler, z = bloomSampler, w = unused
};

void
main()
{
  vec3 hdrColor = texture(scene, texCoords).rgb;
  vec3 bloomColor = texture(bloomBlur, texCoords).rgb;

  // Blend bloom with scene
  vec3 result = mix(hdrColor, bloomColor, 0.04);

  // Apply exposure and tonemapping
  result *= resolution.z; // exposure
  result = vec3(1.0) - exp(-result);

  // Gamma correction
  const float gamma = 2.2;
  result = pow(result, vec3(1.0 / gamma));

  FragColor = vec4(result, 1.0);
}
