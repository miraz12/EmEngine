#version 300 es
// =============================================================================
// Shader: irradiance.frag
// Purpose: Generate diffuse irradiance map via hemisphere convolution
// Method: Numerical integration over hemisphere using spherical coordinates
// =============================================================================
precision highp float;
out vec4 FragColor;
in vec3 worldPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

// =============================================================================
// MAIN
// =============================================================================
void
main()
{
  vec3 N = normalize(worldPos);
  vec3 irradiance = vec3(0.0);

  // Build tangent-space basis for hemisphere sampling
  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = normalize(cross(up, N));
  up = normalize(cross(N, right));

  float sampleDelta = 0.025;
  float nrSamples = 0.0;

  for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
    for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
      vec3 tangentSample =
        vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));

      vec3 sampleVec =
        tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

      irradiance +=
        texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
      nrSamples++;
    }
  }

  irradiance = PI * irradiance * (1.0 / float(nrSamples));
  FragColor = vec4(irradiance, 1.0);
}
