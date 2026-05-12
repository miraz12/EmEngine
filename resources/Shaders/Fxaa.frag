#version 300 es
// =============================================================================
// Shader: Fxaa.frag
// Purpose: Fast approximate anti-aliasing via edge-directed blur
// =============================================================================
precision highp float;

out vec4 FragColor;
in vec2 texCoords;

// Sampler remains as regular uniform (cannot be in UBO)
uniform sampler2D scene;

// PostProcess UBO (binding point 5)
layout(std140) uniform PostProcessData
{
  vec4 resolution;      // xy = resolution, z = exposure, w = filterRadius
  ivec4 postConfig;     // x = mipLevel, y = sceneSampler, z = bloomSampler, w = unused
};

// =============================================================================
// ACES FILMIC TONE MAPPING (Stephen Hill's approximation)
// =============================================================================
vec3 ACESFilm(vec3 x) {
  const float a = 2.51;
  const float b = 0.03;
  const float c = 2.43;
  const float d = 0.59;
  const float e = 0.14;
  return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 tonemap(vec3 hdr) {
  float exposure = resolution.z;
  vec3 mapped = ACESFilm(hdr * exposure);
  // Gamma correction
  return pow(mapped, vec3(1.0 / 2.2));
}

// =============================================================================
// FXAA QUALITY SETTINGS
// =============================================================================
const float FXAA_SPAN_MAX = 8.0;
const float FXAA_REDUCE_MUL = 1.0 / 8.0;
const float FXAA_REDUCE_MIN = 1.0 / 128.0;

// =============================================================================
// MAIN
// =============================================================================
void
main()
{
  vec2 texOffset = 1.0 / resolution.xy;

  // Sample 5-tap cross pattern and tone map each sample
  vec3 rgbNW = tonemap(texture(scene, texCoords + vec2(-1.0, -1.0) * texOffset).xyz);
  vec3 rgbNE = tonemap(texture(scene, texCoords + vec2(1.0, -1.0) * texOffset).xyz);
  vec3 rgbSW = tonemap(texture(scene, texCoords + vec2(-1.0, 1.0) * texOffset).xyz);
  vec3 rgbSE = tonemap(texture(scene, texCoords + vec2(1.0, 1.0) * texOffset).xyz);
  vec3 rgbM = tonemap(texture(scene, texCoords).xyz);

  vec3 luma = vec3(0.299, 0.587, 0.114);
  float lumaNW = dot(rgbNW, luma);
  float lumaNE = dot(rgbNE, luma);
  float lumaSW = dot(rgbSW, luma);
  float lumaSE = dot(rgbSE, luma);
  float lumaM = dot(rgbM, luma);

  float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
  float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

  // Compute edge direction from luminance gradients
  vec2 dir;
  dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
  dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

  float dirReduce =
    max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
  float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

  dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
            max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) *
        texOffset;

  // Sample along edge direction (tone map each sample)
  vec3 rgbA = 0.5 * (tonemap(texture(scene, texCoords + dir * (1.0 / 3.0 - 0.5)).xyz) +
                     tonemap(texture(scene, texCoords + dir * (2.0 / 3.0 - 0.5)).xyz));

  vec3 rgbB = rgbA * 0.5 + 0.25 * (tonemap(texture(scene, texCoords + dir * -0.5).xyz) +
                                   tonemap(texture(scene, texCoords + dir * 0.5).xyz));

  float lumaB = dot(rgbB, luma);

  // Choose narrow or wide blur based on luminance range
  if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
    FragColor = vec4(rgbA, 1.0);
  } else {
    FragColor = vec4(rgbB, 1.0);
  }
}
