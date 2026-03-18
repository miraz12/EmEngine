#version 300 es
// =============================================================================
// Shader: extractBright.frag
// Purpose: Extract bright pixels above luminance threshold for bloom
// =============================================================================
precision highp float;

in vec2 texCoords;
layout(location = 0) out vec4 FragColor;

uniform sampler2D scene;

void
main()
{
  vec3 hdrColor = texture(scene, texCoords).rgb;
  float brightness = dot(hdrColor, vec3(0.2126, 0.7152, 0.0722));

  if (brightness > 1.0) {
    FragColor = vec4(hdrColor, 1.0);
  } else {
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
  }
}
