#version 300 es
// =============================================================================
// Shader: shadow.frag
// Purpose: Depth-only fragment shader for shadow map generation
// =============================================================================
precision highp float;

void
main()
{
  // Depth-only pass: gl_FragDepth is automatically written
  // No manual depth write needed (commented line below is redundant)
  // gl_FragDepth = gl_FragCoord.z;
}
