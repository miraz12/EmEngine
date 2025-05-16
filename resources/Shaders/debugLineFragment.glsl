#version 300 es
precision highp float;

// Input from vertex shader
in vec3 fragColor;

// Output color
out vec4 outputColor;

void main() {
    // Just use the color passed from the vertex shader
    outputColor = vec4(fragColor, 1.0);
}