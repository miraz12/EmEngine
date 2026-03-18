#version 300 es
// =============================================================================
// Shader: shadow.vert
// Purpose: Transform vertices to light space for shadow map generation
// =============================================================================

layout(location = 0) in vec3 POSITION;  // Vertex position
layout(location = 4) in vec4 JOINTS_0;  // Bone indices (4 influences)
layout(location = 5) in vec4 WEIGHTS_0; // Bone weights (4 influences, sum=1)

uniform mat4 modelMatrix;      // Model transform
uniform mat4 lightSpaceMatrix; // Light view-projection matrix
uniform bool is_skinned;       // Enable skeletal animation
uniform sampler2D
  jointMats; // Bone transformation matrices (4 columns per bone)

// Fetch bone transformation matrix from texture
// Bone matrices stored as 4 consecutive texels (columns)
// Params: boneIdx (bone index)
// Returns: 4×4 transformation matrix for the bone
mat4
getBoneMatrix(int boneIdx)
{
  return mat4(texelFetch(jointMats, ivec2(0, boneIdx), 0),
              texelFetch(jointMats, ivec2(1, boneIdx), 0),
              texelFetch(jointMats, ivec2(2, boneIdx), 0),
              texelFetch(jointMats, ivec2(3, boneIdx), 0));
}

void
main()
{
  vec4 worldPos = vec4(POSITION, 1.0);
  if (is_skinned) {
    mat4 skinMat = WEIGHTS_0.x * getBoneMatrix(int(JOINTS_0.x)) +
                   WEIGHTS_0.y * getBoneMatrix(int(JOINTS_0.y)) +
                   WEIGHTS_0.z * getBoneMatrix(int(JOINTS_0.z)) +
                   WEIGHTS_0.w * getBoneMatrix(int(JOINTS_0.w));
    worldPos = skinMat * vec4(POSITION, 1.0);
  }

  gl_Position = lightSpaceMatrix * modelMatrix * worldPos;
}
