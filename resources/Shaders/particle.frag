#version 300 es
// =============================================================================
// Shader: particle.frag
// Purpose: Render particle billboard with color or texture
// =============================================================================
precision highp float;

in vec2 texCoords;
in vec4 particleColor;

layout(location = 0) out vec4 FragColor;

uniform sampler2D sprite;

void
main()
{
  // TODO: Enable texture sampling for textured particles
  // FragColor = texture(sprite, texCoords) * particleColor;

  // Currently: solid color particles only
  FragColor = particleColor;
}
