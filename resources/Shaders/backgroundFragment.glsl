#version 300 es
precision highp float;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColorBright;
in vec3 WorldPos;

uniform samplerCube environmentMap;

void main()
{
    vec3 envColor = texture(environmentMap, WorldPos).rgb;

    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2));

    FragColor = vec4(envColor, 1.0);
    // check whether result is higher than some threshold, if so, output as bloom threshold color
    float brightness = dot(envColor, vec3(0.9, 0.9, 0.9));
    if(brightness > 1.0) {
       FragColorBright = vec4(envColor, 1.0);
    }
}
