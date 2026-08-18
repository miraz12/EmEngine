#version 300 es
// =============================================================================
// Shader: shadowDebugView.frag
// Purpose: Visualize one layer of a cascaded shadow map's raw depth as
//          grayscale, for visual regression testing. Not used at runtime.
// =============================================================================
precision highp float;
precision highp sampler2DArray;

out vec4 FragColor;
in vec2 texCoords;

// Plain (non-comparison) sampler on purpose: pbrLight.frag binds this same
// texture as sampler2DArrayShadow for a PCF shadow factor. Raw depth needs a
// non-comparison sampler instead.
uniform sampler2DArray depthMapArray;
uniform int cascadeLayer;

void
main()
{
  float depth = texture(depthMapArray, vec3(texCoords, float(cascadeLayer))).r;
  FragColor = vec4(vec3(depth), 1.0);
}
