#version 300 es
// =============================================================================
// Shader: cubeMap.vert
// Purpose: Generic vertex shader for rendering to cubemap faces
// =============================================================================

layout(location = 0) in vec3 aPos;

out vec3 worldPos;

// Note: CubeMapPass renders to cubemap faces with per-face view/projection
// matrices, so it uses uniforms instead of CameraData UBO. The UBO contains
// the main camera's matrices, but cubemap generation needs 6 different views.
uniform mat4 projection;
uniform mat4 view;

void
main()
{
  worldPos = aPos;
  gl_Position = projection * view * vec4(worldPos, 1.0);
}
