#version 300 es
// =============================================================================
// Shader: background.vert
// Purpose: Render skybox at infinite distance
// =============================================================================
layout(location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void
main()
{
  WorldPos = aPos;

  // Remove translation from view matrix for infinite distance
  mat4 rotView = mat4(mat3(view));
  vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);

  // Set depth to far plane
  gl_Position = clipPos.xyww;
}
