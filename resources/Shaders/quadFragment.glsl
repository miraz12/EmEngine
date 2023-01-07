#version 300 es
precision highp float;

in vec2 texCoords;
in vec4 pos;


uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

out vec4 FragColor;

void main()
{
    vec3 FragPos = texture(gPosition, texCoords).rgb;
    vec3 Normal = texture(gNormal, texCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, texCoords).rgb;
    FragColor = vec4(Diffuse, 1.0);
}
