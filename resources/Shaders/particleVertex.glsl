#version 300 es
precision highp float;

layout (location = 0) in vec3 POSITION;
layout (location = 1) in vec2 TEXCOORD_0;
out vec2 texCoords;

out vec4 particleColor;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 particlePos;
uniform vec4 color;

void main() {
    float scale = 10.0f;
    particleColor = color;

    vec3 camUp = normalize(vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1])) * POSITION.y * 0.01;
    vec3 camRight = normalize(vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0])) * POSITION.x * 0.01;

    vec3 position = particlePos + camRight + camUp;
    gl_Position = projMatrix * viewMatrix * vec4(position, 1.0);
}
