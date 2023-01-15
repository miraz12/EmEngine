#version 300 es

// If inputs change, also update SimpleShaderProgram::setupVertexAttributePointers to match
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec2 inTexCoords;

// If uniforms change, also update SimpleShaderProgram to match
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec4 color;
out vec2 texCoords;
out vec3 normal;

void main() {
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(inPosition.xyz, 1.0);
    normal = (inverse(transpose(viewMatrix * modelMatrix)) * vec4(normalize(inNormal), 0.0)).xyz;
    texCoords = inTexCoords;
}
