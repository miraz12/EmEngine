#version 300 es
precision highp float;

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D scene;


void main() {
    vec3 hdrColor = texture(scene, texCoord).rgb;
    float brightness = dot(hdrColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
       FragColor = vec4(hdrColor, 1.0);
    }
}
