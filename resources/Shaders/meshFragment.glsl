#version 300 es
precision highp float;

in vec3 position;
in vec2 texCoords;
in vec3 normal;

// If uniforms change, also update SimpleShaderProgram to match
uniform sampler2D texture0;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

mat4 thresholdMatrix = mat4(
    1.0, 9.0, 3.0, 11.0,
    13.0, 5.0, 15.0, 7.0,
    4.0, 12.0, 2.0, 10.0,
    16.0, 8.0, 14.0, 6.0
    );

void main()
{
    gPosition = position;
    gAlbedo = texture(texture0, texCoords);
    gNormal = normal;
}
