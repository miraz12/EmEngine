#version 300 es
precision highp float;

in vec3 pPosition;
in vec2 pTexCoords;
in vec3 pNormal;
in vec3 pTangent;

// If uniforms change, also update SimpleShaderProgram to match
uniform sampler2D textures[5];


layout (location = 0) out vec4 gPositionAo;
layout (location = 1) out vec4 gNormalMetal;
layout (location = 2) out vec4 gAlbedoRough;

mat4 thresholdMatrix = mat4(
    1.0, 9.0, 3.0, 11.0,
    13.0, 5.0, 15.0, 7.0,
    4.0, 12.0, 2.0, 10.0,
    16.0, 8.0, 14.0, 6.0
);

vec3 CalcBumpedNormal() {
    vec3 normal = normalize(pNormal);
    vec3 tangent = normalize(pTangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(tangent, normal);
    vec3 bumpMapNormal = texture(textures[4], pTexCoords).xyz;
    bumpMapNormal = 2.0 * bumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 newNormal;
    mat3 TBN = mat3(tangent, bitangent, normal);
    newNormal = TBN * bumpMapNormal;
    newNormal = normalize(newNormal);
    return newNormal;
}

void main()
{
    float threshold = thresholdMatrix[int(floor(mod(gl_FragCoord.x, 4.0)))][int(floor(mod(gl_FragCoord.y, 4.0)))] / 17.0;
    if (threshold >= texture(textures[0], pTexCoords).a) {
        discard;
    }
    float metal = texture(textures[1], pTexCoords).b;
    float roughness = texture(textures[1], pTexCoords).g;
    float ao = texture(textures[3], pTexCoords).r;
    gPositionAo = vec4(pPosition, ao);
    gAlbedoRough = vec4(texture(textures[0], pTexCoords).rgb, roughness);
    if (length(pTangent) > 0.0) {
        gNormalMetal = vec4(CalcBumpedNormal(), metal);
    } else {
        gNormalMetal = vec4(normalize(pNormal * texture(textures[4], pTexCoords).xyz), metal);
    }

}
