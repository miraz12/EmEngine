#version 300 es
// =============================================================================
// Shader: prefilter.frag
// Purpose: Generate prefiltered specular environment map for split-sum IBL
// Method: Monte Carlo integration with importance sampling
// =============================================================================
precision highp float;
out vec4 FragColor;
in vec3 worldPos;

uniform samplerCube environmentMap;
uniform float roughness;

const float PI = 3.14159265359;

// =============================================================================
// GGX NORMAL DISTRIBUTION FUNCTION
// =============================================================================

// GGX (Trowbridge-Reitz) normal distribution function
// Defines the distribution of microfacet normals (specular lobe shape)
// Params: N (surface normal), H (halfway vector), roughness (material
// roughness) Returns: Probability density of microfacets aligned with H
float
DistributionGGX(vec3 N, vec3 H, float roughness)
{
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}

// =============================================================================
// LOW-DISCREPANCY SEQUENCE GENERATION
// =============================================================================

// Van Der Corput sequence: generates low-discrepancy samples in [0,1]
// Avoids bitwise operations (not supported in GLSL ES 3.0)
// Params: n (sample index), base (radix, typically 2)
// Returns: Quasi-random value in [0,1] with good stratification
float
VanDerCorput(uint n, uint base)
{
  float invBase = 1.0 / float(base);
  float denom = 1.0;
  float result = 0.0;

  for (uint i = 0u; i < 32u; ++i) {
    if (n > 0u) {
      denom = mod(float(n), 2.0);
      result += denom * invBase;
      invBase = invBase / 2.0;
      n = uint(float(n) / 2.0);
    }
  }

  return result;
}

// Hammersley sequence: 2D low-discrepancy point set for Monte Carlo integration
// Combines uniform distribution (x-axis) with Van Der Corput (y-axis)
// Params: i (sample index), N (total sample count)
// Returns: 2D sample point in [0,1]² with low-discrepancy properties
vec2
HammersleyNoBitOps(uint i, uint N)
{
  return vec2(float(i) / float(N), VanDerCorput(i, 2u));
}

// =============================================================================
// IMPORTANCE SAMPLING
// =============================================================================

// GGX importance sampling: generates sample directions biased toward specular
// lobe Uses spherical coordinates with GGX distribution to minimize variance
// Params: Xi (2D random sample [0,1]²), N (surface normal), roughness (material
// roughness) Returns: Normalized sample direction (halfway vector H) in world
// space
vec3
ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
  float a = roughness * roughness;

  float phi = 2.0 * PI * Xi.x;
  float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

  // from spherical coordinates to cartesian coordinates - halfway vector
  vec3 H;
  H.x = cos(phi) * sinTheta;
  H.y = sin(phi) * sinTheta;
  H.z = cosTheta;

  // from tangent-space H vector to world-space sample vector
  vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
  vec3 tangent = normalize(cross(up, N));
  vec3 bitangent = cross(N, tangent);

  vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
  return normalize(sampleVec);
}

// =============================================================================
// MAIN
// =============================================================================
void
main()
{
  vec3 N = normalize(worldPos);
  vec3 R = N;
  vec3 V = R;

  const uint SAMPLE_COUNT = 1024u;
  vec3 prefilteredColor = vec3(0.0);
  float totalWeight = 0.0;

  for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
    vec2 Xi = HammersleyNoBitOps(i, SAMPLE_COUNT);
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);
    vec3 L = normalize(2.0 * dot(V, H) * H - V);

    float NdotL = max(dot(N, L), 0.0);
    if (NdotL > 0.0) {
      float D = DistributionGGX(N, H, roughness);
      float NdotH = max(dot(N, H), 0.0);
      float HdotV = max(dot(H, V), 0.0);
      float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

      float resolution = 512.0;
      float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
      float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
      float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

      prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
      totalWeight += NdotL;
    }
  }

  prefilteredColor = prefilteredColor / totalWeight;
  FragColor = vec4(prefilteredColor, 1.0);
}
