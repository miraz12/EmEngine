#version 300 es
precision highp float;

in vec2 texCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

out vec4 FragColor;

void main()
{
    vec3 FragPos = texture(gPosition, texCoords).rgb;
    vec3 Normal = texture(gNormal, texCoords).rgb;
    vec3 diffuse = texture(gAlbedoSpec, texCoords).rgb;
    FragColor = vec4(diffuse, 1.0);
}
