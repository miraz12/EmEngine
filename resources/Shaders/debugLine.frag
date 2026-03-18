#version 300 es
// =============================================================================
// Shader: debugLine.frag
// Purpose: Render unlit colored debug lines
// =============================================================================
precision highp float;

in vec3 fragColor;

out vec4 FragColor;

void
main()
{
  FragColor = vec4(fragColor, 1.0);
}