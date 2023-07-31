#version 300 es
precision highp float;

in vec2 texCoords;
in vec4 particleColor;

layout (location = 0) out vec4 FragColor;

uniform sampler2D sprite;

void main() {
    // fragColor = (texture(sprite, texCoords) * particleColor);
    FragColor = particleColor;
}
