#version 300 es
// =============================================================================
// Shader: debugLine.vert
// Purpose: Render debug visualization lines
// =============================================================================
precision highp float;

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 COLOR;

out vec3 fragColor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void
main()
{
  gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(POSITION, 1.0);
  fragColor = COLOR;
}
