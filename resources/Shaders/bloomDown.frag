#version 300 es
// =============================================================================
// Shader: bloomDown.frag
// Purpose: Downsample using 13-tap filter with Karis average
// =============================================================================
precision highp float;

uniform sampler2D srcTexture;
uniform vec2 srcResolution;
uniform int mipLevel;

in vec2 texCoords;
layout(location = 0) out vec3 downsample;

// =============================================================================
// HELPER FUNCTIONS FOR KARIS AVERAGE
// =============================================================================

// Component-wise power for gamma correction
vec3
PowVec3(vec3 v, float p)
{
  return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

// Convert linear HDR color to sRGB for luma calculation
const float invGamma = 1.0 / 2.2;
vec3
ToSRGB(vec3 v)
{
  return PowVec3(v, invGamma);
}

// Calculate perceptual luminance in sRGB space
float
sRGBToLuma(vec3 col)
{
  return dot(col, vec3(0.299, 0.587, 0.114));
}

// Karis average: weight samples by inverse luminance to suppress fireflies
float
KarisAverage(vec3 col)
{
  float luma = sRGBToLuma(ToSRGB(col)) * 0.25;
  return 1.0 / (1.0 + luma);
}

// =============================================================================
// MAIN
// =============================================================================

// NOTE: This is the readable version of this shader. It will be optimized!
void
main()
{
  vec2 srcTexelSize = 1.0 / srcResolution;
  float x = srcTexelSize.x;
  float y = srcTexelSize.y;

  // Take 13 samples around current texel:
  // a - b - c
  // - j - k -
  // d - e - f
  // - l - m -
  // g - h - i
  // === ('e' is the current texel) ===
  vec3 a =
    texture(srcTexture, vec2(texCoords.x - 2.0 * x, texCoords.y + 2.0 * y)).rgb;
  vec3 b = texture(srcTexture, vec2(texCoords.x, texCoords.y + 2.0 * y)).rgb;
  vec3 c =
    texture(srcTexture, vec2(texCoords.x + 2.0 * x, texCoords.y + 2.0 * y)).rgb;

  vec3 d = texture(srcTexture, vec2(texCoords.x - 2.0 * x, texCoords.y)).rgb;
  vec3 e = texture(srcTexture, vec2(texCoords.x, texCoords.y)).rgb;
  vec3 f = texture(srcTexture, vec2(texCoords.x + 2.0 * x, texCoords.y)).rgb;

  vec3 g =
    texture(srcTexture, vec2(texCoords.x - 2.0 * x, texCoords.y - 2.0 * y)).rgb;
  vec3 h = texture(srcTexture, vec2(texCoords.x, texCoords.y - 2.0 * y)).rgb;
  vec3 i =
    texture(srcTexture, vec2(texCoords.x + 2.0 * x, texCoords.y - 2.0 * y)).rgb;

  vec3 j = texture(srcTexture, vec2(texCoords.x - x, texCoords.y + y)).rgb;
  vec3 k = texture(srcTexture, vec2(texCoords.x + x, texCoords.y + y)).rgb;
  vec3 l = texture(srcTexture, vec2(texCoords.x - x, texCoords.y - y)).rgb;
  vec3 m = texture(srcTexture, vec2(texCoords.x + x, texCoords.y - y)).rgb;

  // 13-tap kernel with weighted distribution (energy-preserving)
  // Groups overlap to maintain energy conservation

  // Apply Karis average on mip 0 to prevent fireflies
  vec3 groups[5];
  switch (mipLevel) {
    case 0:
      // First downsample: apply Karis average to suppress fireflies
      groups[0] = (a + b + d + e) * (0.125 / 4.0);
      groups[1] = (b + c + e + f) * (0.125 / 4.0);
      groups[2] = (d + e + g + h) * (0.125 / 4.0);
      groups[3] = (e + f + h + i) * (0.125 / 4.0);
      groups[4] = (j + k + l + m) * (0.5 / 4.0);
      groups[0] *= KarisAverage(groups[0]);
      groups[1] *= KarisAverage(groups[1]);
      groups[2] *= KarisAverage(groups[2]);
      groups[3] *= KarisAverage(groups[3]);
      groups[4] *= KarisAverage(groups[4]);
      downsample = groups[0] + groups[1] + groups[2] + groups[3] + groups[4];
      downsample = max(downsample, 0.0001);
      break;
    default:
      // Subsequent mips: standard weighted downsample
      downsample = e * 0.125;
      downsample += (a + c + g + i) * 0.03125;
      downsample += (b + d + f + h) * 0.0625;
      downsample += (j + k + l + m) * 0.125;
      break;
  }
}
