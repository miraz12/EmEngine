#version 300 es
// =============================================================================
// Shader: pbrMesh.frag
// Purpose: Fill G-buffer with PBR material properties
// =============================================================================
precision highp float;

in vec3 pPosition;  // World-space position
in vec2 pTexCoords; // Texture coordinates
in vec3 pNormal;    // Vertex normal (for TBN construction)

// ============================================================
// UNIFORMS
// ============================================================

// Material texture array (glTF 2.0 standard)
// Index 0: Base color (RGBA)
// Index 1: Metallic/Roughness (B=metallic, G=roughness per glTF spec)
// Index 2: Emissive (RGB)
// Index 3: Occlusion (R channel)
// Index 4: Normal map (RGB tangent-space)
uniform sampler2D textures[5];

// ============================================================
// UNIFORM BUFFER OBJECTS
// ============================================================

// Material UBO (binding point 3)
// Matches gfx::MaterialUBO struct (64 bytes, std140 layout)
layout(std140) uniform MaterialData
{
  vec4 baseColorFactor;    // xyz = color, w = alpha
  vec4 emissiveFactor;     // xyz = emissive, w = unused
  vec4 pbrFactors;         // x = roughness, y = metallic, z = alphaCutoff, w = unused
  ivec4 materialConfig;    // x = materialFlags, y = alphaMode, zw = unused
};

// ============================================================
// G-BUFFER OUTPUTS
// ============================================================

layout(location = 0) out vec4 gPositionAo;  // RGB: position, A: AO
layout(location = 1) out vec4 gNormalMetal; // RGB: normal, A: metallic
layout(location = 2) out vec4 gAlbedoRough; // RGB: albedo, A: roughness
layout(location = 3) out vec4 gEmissive;    // RGB: emissive, A: unused

// Bayer 4×4 dithering matrix for ordered dithering (alpha blending)
// Values 1-16 normalized to [0-1] range by dividing by 16
mat4 thresholdMatrix = mat4(1.0,
                            9.0,
                            3.0,
                            11.0,
                            13.0,
                            5.0,
                            15.0,
                            7.0,
                            4.0,
                            12.0,
                            2.0,
                            10.0,
                            16.0,
                            8.0,
                            14.0,
                            6.0);

// ============================================================
// NORMAL MAPPING
// ============================================================

// Compute world-space normal with tangent-space normal mapping
// Constructs TBN (tangent-bitangent-normal) basis using screen-space
// derivatives Handles cases with/without normal maps and back-facing surfaces
// Returns: World-space normal vector
vec3
getNormal()
{
  vec3 tangentNormal = texture(textures[4], pTexCoords).xyz * 2.0 - 1.0;

  vec2 UV = pTexCoords;
  vec2 uv_dx = dFdx(UV);
  vec2 uv_dy = dFdy(UV);

  if (length(uv_dx) + length(uv_dy) <= 1e-6) {
    uv_dx = vec2(1.0, 0.0);
    uv_dy = vec2(0.0, 1.0);
  }

  vec3 t_ = (uv_dy.t * dFdx(pPosition) - uv_dx.t * dFdy(pPosition)) /
            (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);

  vec3 n, t, b, ng;

  if ((materialConfig.x & (1 << 4)) > 0) {
    ng = normalize(pNormal);
    t = normalize(t_ - ng * dot(ng, t_));
    b = cross(ng, t);
  } else {
    ng = normalize(cross(dFdx(pPosition), dFdy(pPosition)));
    return ng;
  }

  // For a back-facing surface, the tangential basis vectors are negated.
  if (gl_FrontFacing == false) {
    t *= -1.0;
    b *= -1.0;
    ng *= -1.0;
  }

  return normalize(mat3(t, b, ng) * tangentNormal);
}

void
main()
{
  // Extract material flags for readability
  int matFlags = materialConfig.x;
  int alphaMode = materialConfig.y;
  float roughnessFactor = pbrFactors.x;
  float metallicFactor = pbrFactors.y;
  float alphaCutoff = pbrFactors.z;

  // Cache texture samples to avoid redundant lookups
  vec4 baseColorSample =
    (matFlags & (1 << 0)) > 0 ? texture(textures[0], pTexCoords) : vec4(1.0);
  vec4 metallicRoughnessSample =
    (matFlags & (1 << 1)) > 0 ? texture(textures[1], pTexCoords) : vec4(1.0);

  float metal = metallicFactor;
  vec4 baseRough = vec4(baseColorFactor.xyz, roughnessFactor);
  if ((matFlags & (1 << 0)) > 0) {
    baseRough.rgb *= baseColorSample.rgb;
  }
  if ((matFlags & (1 << 1)) > 0) {
    metal *= metallicRoughnessSample.b;       // glTF: blue = metallic
    baseRough.a *= metallicRoughnessSample.g; // glTF: green = roughness
  }
  vec4 emissive = vec4(emissiveFactor.xyz, 1.0);
  if ((matFlags & (1 << 2)) > 0) {
    emissive *= texture(textures[2], pTexCoords);
  }
  float ao = 1.0;
  if ((matFlags & (1 << 3)) > 0) {
    ao = texture(textures[3], pTexCoords).r;
  }

  if (alphaMode == 1) {
    // MASK: hard cutoff, no dithering
    if (baseColorSample.a < alphaCutoff) {
      discard;
    }
  } else if (alphaMode == 0) {
    // BLEND: ordered dithering (deferred pipeline has no blending in G-buffer)
    float threshold = thresholdMatrix[int(floor(mod(gl_FragCoord.x, 4.0)))]
                                     [int(floor(mod(gl_FragCoord.y, 4.0)))] /
                      16.0;
    if (threshold >= baseColorSample.a) {
      discard;
    }
  }

  gNormalMetal = vec4(getNormal(), metal);
  gPositionAo = vec4(pPosition, ao);
  gAlbedoRough = baseRough;
  gEmissive = emissive;
}
