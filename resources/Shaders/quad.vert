#version 300 es
// =============================================================================
// Shader: quad.vert
// Purpose: Generic fullscreen quad vertex shader for post-processing
// =============================================================================

layout(location = 0) in vec3 inPosition;  // NDC positions
layout(location = 1) in vec2 inTexCoords; // Texture coordinates

out vec2 texCoords;

void
main()
{
  gl_Position = vec4(inPosition, 1.0);
  texCoords = inTexCoords;
}
