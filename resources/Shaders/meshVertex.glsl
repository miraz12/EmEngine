#version 300 es

layout (location = 0) in vec3 POSITION;
layout (location = 1) in vec3 NORMAL;
layout (location = 2) in vec4 TANGENT;
layout (location = 3) in vec2 TEXCOORD_0;
layout (location = 4) in vec4 JOINTS_0;
layout (location = 5) in vec4 WEIGHTS_0;


uniform mat4 modelMatrix;
uniform mat4 meshMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform float is_skinned;

uniform sampler2D jointMats;

out vec3 pPosition;
out vec2 pTexCoords;
out vec3 pNormal;

mat4 getBoneMatrix(int boneIdx) {
  return mat4(
    texelFetch(jointMats, ivec2(0, boneIdx), 0),
    texelFetch(jointMats, ivec2(1, boneIdx), 0),
    texelFetch(jointMats, ivec2(2, boneIdx), 0),
    texelFetch(jointMats, ivec2(3, boneIdx), 0));
}

void main() {
  vec4 worldPos = vec4(POSITION, 1.0);
  if(is_skinned >= 0.05f) {

    mat4 skinMat =
        WEIGHTS_0.x * getBoneMatrix(int(JOINTS_0.x)) +
        WEIGHTS_0.y * getBoneMatrix(int(JOINTS_0.y)) +
        WEIGHTS_0.z * getBoneMatrix(int(JOINTS_0.z)) +
        WEIGHTS_0.w * getBoneMatrix(int(JOINTS_0.w));
        worldPos = skinMat * vec4(POSITION, 1.0);
  }

  gl_Position = projMatrix * viewMatrix * modelMatrix * worldPos;
  pPosition = (modelMatrix * worldPos).xyz;
  mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

  pNormal =  normalize(normalMatrix * NORMAL);
  pTexCoords = TEXCOORD_0;
}
