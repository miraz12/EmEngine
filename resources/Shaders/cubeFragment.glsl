#version 300 es
precision highp float;

in vec3 pPosition;
uniform sampler2D equirectangularMap;
out vec4 FragColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = SampleSphericalMap(normalize(pPosition));
    vec3 color = texture(equirectangularMap, uv).rgb;
    FragColor = vec4(vec3(1.0, 0.0, 0.0), 1.0);
}
