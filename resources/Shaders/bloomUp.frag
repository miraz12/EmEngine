#version 300 es
// =============================================================================
// Shader: bloomUp.frag
// Purpose: Upsample using 3×3 tent filter
// =============================================================================
precision highp float;

uniform sampler2D srcTexture;

// PostProcess UBO (binding point 5)
layout(std140) uniform PostProcessData
{
  vec4 resolution;      // xy = resolution, z = exposure, w = filterRadius
  ivec4 postConfig;     // x = mipLevel, y = sceneSampler, z = bloomSampler, w = unused
};

in vec2 texCoords;
layout(location = 0) out vec3 upsample;

void
main()
{
  float x = resolution.w; // filterRadius
  float y = resolution.w;

  // 9-tap sampling pattern:
  // a - b - c
  // d - e - f
  // g - h - i
  // ('e' is the current texel center)
  vec3 a = texture(srcTexture, vec2(texCoords.x - x, texCoords.y + y)).rgb;
  vec3 b = texture(srcTexture, vec2(texCoords.x, texCoords.y + y)).rgb;
  vec3 c = texture(srcTexture, vec2(texCoords.x + x, texCoords.y + y)).rgb;

  vec3 d = texture(srcTexture, vec2(texCoords.x - x, texCoords.y)).rgb;
  vec3 e = texture(srcTexture, vec2(texCoords.x, texCoords.y)).rgb;
  vec3 f = texture(srcTexture, vec2(texCoords.x + x, texCoords.y)).rgb;

  vec3 g = texture(srcTexture, vec2(texCoords.x - x, texCoords.y - y)).rgb;
  vec3 h = texture(srcTexture, vec2(texCoords.x, texCoords.y - y)).rgb;
  vec3 i = texture(srcTexture, vec2(texCoords.x + x, texCoords.y - y)).rgb;

  // 3×3 tent filter (Gaussian approximation):
  //  1   | 1 2 1 |
  // -- * | 2 4 2 |
  // 16   | 1 2 1 |
  upsample = e * 4.0;
  upsample += (b + d + f + h) * 2.0;
  upsample += (a + c + g + i);
  upsample *= 1.0 / 16.0;
}
