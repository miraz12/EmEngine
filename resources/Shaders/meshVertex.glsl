#version 300 es

// If inputs change, also update SimpleShaderProgram::setupVertexAttributePointers to match
layout (location = 0) in vec3 POSITION;
layout (location = 1) in vec3 NORMAL;
layout (location = 2) in vec4 TANGENT;
layout (location = 3) in vec2 TEXCOORD_0;

// If uniforms change, also update SimpleShaderProgram to match
uniform mat4 modelMatrix;
uniform mat4 meshMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec3 pPosition;
out vec2 pTexCoords;
out vec3 pNormal;
out vec3 pTangent;
out vec3 pBiTangent;
out mat3 pTBN;

void main() {
    gl_Position = projMatrix * viewMatrix * modelMatrix * meshMatrix * vec4(POSITION, 1.0);
    pPosition = (modelMatrix * meshMatrix * vec4(POSITION.xyz, 1.0)).xyz;
    mat4 normalMatrix = transpose(inverse(modelMatrix * meshMatrix));

    pNormal =  normalize(vec3(normalMatrix * vec4(NORMAL, 0.0)));
    pTangent =  normalize(vec3(normalMatrix * vec4(TANGENT.xyz, 0.0)));
    pBiTangent = cross(pNormal, pTangent) * TANGENT.w;
    pTBN = mat3(pTangent, pBiTangent, pNormal);
    pTexCoords = TEXCOORD_0;
}
