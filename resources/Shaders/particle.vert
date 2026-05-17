#version 300 es
// =============================================================================
// Shader: particle.vert
// Purpose: Render camera-facing particle billboards (instanced)
// =============================================================================
precision highp float;

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXCOORD_0;

// Per-instance attributes (binding 1, divisor=1)
layout(location = 2) in vec3 iParticlePos;
layout(location = 3) in vec4 iColor;

out vec2 texCoords;
out vec4 particleColor;

uniform mat4 viewMatrix;
uniform mat4 projMatrix;

void
main()
{
  particleColor = iColor;

  // Extract camera vectors for billboarding
  vec3 camUp =
    normalize(vec3(viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1])) *
    POSITION.y * 0.01;
  vec3 camRight =
    normalize(vec3(viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0])) *
    POSITION.x * 0.01;

  vec3 position = iParticlePos + camRight + camUp;
  gl_Position = projMatrix * viewMatrix * vec4(position, 1.0);
}
