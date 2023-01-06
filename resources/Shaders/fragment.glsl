#version 300 es
precision highp float;

in vec4 color;
in vec2 texCoords;
in vec3 normal;

// If uniforms change, also update SimpleShaderProgram to match
uniform sampler2D texture0;
uniform int useTexture;

out vec4 FragColor;

mat4 thresholdMatrix = mat4(
    1.0, 9.0, 3.0, 11.0,
    13.0, 5.0, 15.0, 7.0,
    4.0, 12.0, 2.0, 10.0,
    16.0, 8.0, 14.0, 6.0
    );

void main()
{
    float lum = max(dot(normal, normalize(vec3(3.0, 10.0, -5.0))), 0.0);
    FragColor = texture(texture0, texCoords) * vec4((0.3 + 0.7 * lum) * vec3(1.0, 1.0, 1.0), 1.0);

}
