#version 300 es
// =============================================================================
// Shader: debugLine.vert
// Purpose: Render debug visualization lines
// =============================================================================
precision highp float;

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 COLOR;

out vec3 fragColor;

// Camera UBO (binding point 1)
layout(std140) uniform CameraData
{
  mat4 viewMatrix;
  mat4 projMatrix;
  mat4 viewProjMatrix;
  vec4 cameraPosition; // xyz = position, w = unused
};

uniform mat4 modelMatrix;

void
main()
{
  gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(POSITION, 1.0);
  fragColor = COLOR;
}
