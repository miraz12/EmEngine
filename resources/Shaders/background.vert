#version 300 es
// =============================================================================
// Shader: background.vert
// Purpose: Render skybox at infinite distance
// =============================================================================
layout(location = 0) in vec3 aPos;

// Camera UBO (binding point 1)
layout(std140) uniform CameraData
{
  mat4 viewMatrix;
  mat4 projMatrix;
  mat4 viewProjMatrix;
  vec4 cameraPosition; // xyz = position, w = unused
};

out vec3 WorldPos;

void
main()
{
  WorldPos = aPos;

  // Remove translation from view matrix for infinite distance
  mat4 rotView = mat4(mat3(viewMatrix));
  vec4 clipPos = projMatrix * rotView * vec4(WorldPos, 1.0);

  // Set depth to far plane
  gl_Position = clipPos.xyww;
}
