#version 300 es
precision highp float;

out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;

void main() {
    // to bloom or not to bloom
    vec3 result = vec3(0.0);
    vec3 hdrColor = texture(scene, texCoord).rgb;
    vec3 bloomColor = texture(bloomBlur, texCoord).rgb;
    result = mix(hdrColor, bloomColor, 0.04f);
    // tone mapping
    result = vec3(1.0) - exp(-result * exposure);
    // gamma correction
    const float gamma = 2.2;
    result = pow(result, vec3(1.0 / gamma));

    FragColor = vec4(result, 1.0);
}
