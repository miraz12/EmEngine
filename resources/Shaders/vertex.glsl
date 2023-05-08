#version 300 es

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec4 inColor;
layout (location = 2) in vec2 inTexCoords;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

out vec4 color;
out vec2 texCoords;

void main() {
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(inPosition.xyz, 1.0);
    color = inColor;
    texCoords = inTexCoords;
}
