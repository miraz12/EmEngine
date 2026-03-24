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

  // Sample 5-tap cross pattern
  vec3 rgbNW = texture(scene, texCoords + vec2(-1.0, -1.0) * texOffset).xyz;
  vec3 rgbNE = texture(scene, texCoords + vec2(1.0, -1.0) * texOffset).xyz;
  vec3 rgbSW = texture(scene, texCoords + vec2(-1.0, 1.0) * texOffset).xyz;
  vec3 rgbSE = texture(scene, texCoords + vec2(1.0, 1.0) * texOffset).xyz;
  vec3 rgbM = texture(scene, texCoords).xyz;

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

  // Sample along edge direction
  vec3 rgbA = 0.5 * (texture(scene, texCoords + dir * (1.0 / 3.0 - 0.5)).xyz +
                     texture(scene, texCoords + dir * (2.0 / 3.0 - 0.5)).xyz);

  vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(scene, texCoords + dir * -0.5).xyz +
                                   texture(scene, texCoords + dir * 0.5).xyz);

  float lumaB = dot(rgbB, luma);

  // Choose narrow or wide blur based on luminance range
  if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
    FragColor = vec4(rgbA, 1.0);
  } else {
    FragColor = vec4(rgbB, 1.0);
  }
}
