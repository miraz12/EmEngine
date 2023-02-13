#version 300 es

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

layout (location = 0) in vec3 inPosition;
out vec3 pPosition;

void main() {

    gl_Position = projMatrix * viewMatrix * vec4(inPosition, 1.0);
    pPosition = inPosition;
}
