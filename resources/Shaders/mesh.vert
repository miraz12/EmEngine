#version 300 es
// =============================================================================
// Shader: mesh.vert
// Purpose: Transform mesh vertices and output attributes for G-buffer
// =============================================================================

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location =
         2) in vec4 TANGENT; // Currently unused (TBN built in fragment shader)
layout(location = 3) in vec2 TEXCOORD_0;
layout(location = 4) in vec4 JOINTS_0;  // Bone indices
layout(location = 5) in vec4 WEIGHTS_0; // Bone weights

// Per-instance model matrix
layout(location = 6) in mat4 iModelMatrix;
// Per-instance first jointMats row for this instance's skeleton. Carried
// per instance (not as a uniform) so several skinned entities with
// different skeletons can be drawn in a single instanced call.
layout(location = 10) in float iJointOffset;

// Camera UBO (binding point 1)
layout(std140) uniform CameraData
{
  mat4 viewMatrix;
  mat4 projMatrix;
  mat4 viewProjMatrix;
  vec4 cameraPosition; // xyz = position, w = unused
};

uniform mat4 meshMatrix; // Currently unused
uniform bool is_skinned;
uniform sampler2D jointMats; // Bone matrices texture

out vec3 pPosition;  // World-space position (for lighting)
out vec2 pTexCoords; // Texture coordinates
out vec3 pNormal;    // World-space normal (for TBN fallback)

// Fetch bone transformation matrix (identical to shadow.vert).
// jointMats packs every simultaneously-visible skeleton into contiguous row
// ranges, so boneIdx is biased by this instance's own range to avoid
// reading another entity's skeleton.
mat4
getBoneMatrix(int boneIdx)
{
  int row = int(iJointOffset) + boneIdx;
  return mat4(texelFetch(jointMats, ivec2(0, row), 0),
              texelFetch(jointMats, ivec2(1, row), 0),
              texelFetch(jointMats, ivec2(2, row), 0),
              texelFetch(jointMats, ivec2(3, row), 0));
}

void
main()
{
  mat4 model = iModelMatrix;

  vec4 worldPos = vec4(POSITION, 1.0);
  vec3 skinnedNormal = NORMAL;

  if (is_skinned) {
    mat4 skinMat = WEIGHTS_0.x * getBoneMatrix(int(JOINTS_0.x)) +
                   WEIGHTS_0.y * getBoneMatrix(int(JOINTS_0.y)) +
                   WEIGHTS_0.z * getBoneMatrix(int(JOINTS_0.z)) +
                   WEIGHTS_0.w * getBoneMatrix(int(JOINTS_0.w));
    worldPos = skinMat * vec4(POSITION, 1.0);
    // Transform normal by the skinning matrix (use mat3 to ignore translation)
    skinnedNormal = mat3(skinMat) * NORMAL;
  }

  gl_Position = projMatrix * viewMatrix * model * worldPos;
  pPosition = (model * worldPos).xyz;
  mat3 normalMatrix = transpose(inverse(mat3(model)));

  pNormal = normalize(normalMatrix * skinnedNormal);
  pTexCoords = TEXCOORD_0;
}
