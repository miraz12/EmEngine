#version 300 es
precision highp float;

layout (location = 0) in vec3 POSITION;
layout (location = 1) in vec2 TEXCOORD_0;
out vec2 texCoord;

void main() {
    gl_Position = vec4(POSITION.xy, 0.0, 1.0);
	texCoord = TEXCOORD_0;
}
