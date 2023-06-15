#version 300 es
precision highp float;

layout (location = 0) in vec3 POSITION;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 meshMatrix;

void main() {
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(POSITION, 1.0);
}
