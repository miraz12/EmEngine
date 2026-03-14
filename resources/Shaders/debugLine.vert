#version 300 es
precision highp float;

// Input attributes - position and color
layout (location = 0) in vec3 POSITION;
layout (location = 1) in vec3 COLOR;

// Output to fragment shader
out vec3 fragColor;

// Transformation matrices
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void main() {
    // Transform the vertex position by the model-view-projection matrix
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(POSITION, 1.0);
    
    // Pass color to the fragment shader
    fragColor = COLOR;
}