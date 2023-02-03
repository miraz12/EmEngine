#version 300 es

// If inputs change, also update SimpleShaderProgram::setupVertexAttributePointers to match
layout (location = 0) in vec3 POSITION;
layout (location = 1) in vec3 NORMAL;
layout (location = 2) in vec4 TANGENT;
layout (location = 3) in vec2 TEXCOORD_0;

// If uniforms change, also update SimpleShaderProgram to match
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec3 position;
out vec2 texCoords;
out vec3 normal;

void main() {
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(POSITION, 1.0);
    position = gl_Position.xyz;
    normal =  normalize(NORMAL);
    texCoords = TEXCOORD_0 ;
}
