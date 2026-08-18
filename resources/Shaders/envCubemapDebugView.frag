#version 300 es
// =============================================================================
// Shader: envCubemapDebugView.frag
// Purpose: Visualize one face of the baked IBL environment cubemap for visual
//          regression testing. Samples by direction, like background.frag,
//          rather than reading the face's raw texture memory (which comes
//          out vertically flipped — see CubeMapPass::m_captureViews).
// =============================================================================
precision highp float;

out vec4 FragColor;
in vec3 worldPos;

uniform samplerCube environmentMap;

void
main()
{
  FragColor = vec4(texture(environmentMap, normalize(worldPos)).rgb, 1.0);
}
