#version 300 es

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoords;

out vec2 texCoords;
out vec4 pos;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    pos = gl_Position;
    texCoords = inTexCoords;
}
