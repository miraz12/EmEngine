#version 300 es
precision highp float;

in vec3 pPosition;
in vec2 pTexCoords;
in vec3 pNormal;

// If uniforms change, also update SimpleShaderProgram to match
// 0 = Base color
// 1 = Metal/Roughness
// 2 = Emissive
// 3 = Occlusion
// 4 = Normal
uniform sampler2D textures[5];
uniform int material;
uniform vec3 emissiveFactor;
uniform vec3 baseColorFactor;
uniform float roughnessFactor;
uniform float metallicFactor;


layout (location = 0) out vec4 gPositionAo;
layout (location = 1) out vec4 gNormalMetal;
layout (location = 2) out vec4 gAlbedoRough;
layout (location = 3) out vec4 gEmissive;

mat4 thresholdMatrix = mat4(
    1.0, 9.0, 3.0, 11.0,
    13.0, 5.0, 15.0, 7.0,
    4.0, 12.0, 2.0, 10.0,
    16.0, 8.0, 14.0, 6.0
);

// Find the normal for this fragment
vec3 getNormal() {
	// Perturb normal, see http://www.thetenthplanet.de/archives/1180
	vec3 tangentNormal = texture(textures[4], pTexCoords).xyz * 2.0 - 1.0;

	vec3 q1 = dFdx(pPosition);
	vec3 q2 = dFdy(pPosition);
	vec2 st1 = dFdx(pTexCoords);
	vec2 st2 = dFdy(pTexCoords);

	vec3 N = normalize(pNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

void main() {
    float threshold = thresholdMatrix[int(floor(mod(gl_FragCoord.x, 4.0)))][int(floor(mod(gl_FragCoord.y, 4.0)))] / 17.0;
    if (threshold >= texture(textures[0], pTexCoords).a) {
        discard;
    }

    float metal = metallicFactor;
    vec4 baseRough = vec4(baseColorFactor, roughnessFactor);
    if ((material & (1 << 0)) > 0) {
        baseRough.rgb *= (texture(textures[0], pTexCoords)).rgb;
    }
    if ((material & (1 << 1)) > 0) {
       metal *= texture(textures[1], pTexCoords).b;
       baseRough.a *= texture(textures[1], pTexCoords).g ;
    }
    vec4 emissive = vec4(vec3(emissiveFactor), 1.0);
    if ((material & ( 1 << 2 )) > 0) {
       emissive *= texture(textures[2], pTexCoords);
    }
    float ao = 1.0;
    if ((material & (1 << 3)) > 0) {
        ao = texture(textures[3], pTexCoords).r;
    }

    gNormalMetal = vec4(getNormal(), metal);
    gPositionAo = vec4(pPosition, ao);
    gAlbedoRough = baseRough;
    gEmissive = emissive ;
}
