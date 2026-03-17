#version 300 es
precision highp float;
layout (location = 0) out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

void main() {
    vec3 envColor = texture(environmentMap, WorldPos).rgb;
    FragColor = vec4(envColor, 1.0);
}
