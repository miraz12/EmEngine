#version 300 es

layout (location = 0) in vec3 POSITION;

uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 meshMatrix;


void main()
{
    gl_Position = lightSpaceMatrix * modelMatrix * meshMatrix * vec4(POSITION, 1.0);
}
