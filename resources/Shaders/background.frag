#version 300 es
precision highp float;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColorBright;
in vec3 WorldPos;

uniform samplerCube environmentMap;

void main() {
    vec3 envColor = texture(environmentMap, WorldPos).rgb;
    FragColor = vec4(envColor, 1.0);

    // Extract bright areas for bloom
    float brightness = dot(envColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        FragColorBright = vec4(envColor, 1.0);
    else
        FragColorBright = vec4(0.0, 0.0, 0.0, 1.0);
}
