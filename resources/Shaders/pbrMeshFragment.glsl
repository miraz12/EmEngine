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


vec3 getNormal() {
	vec3 tangentNormal = texture(textures[4], pTexCoords).xyz * 2.0 - 1.0;

    vec2 UV = pTexCoords;
    vec2 uv_dx = dFdx(UV);
    vec2 uv_dy = dFdy(UV);

    if (length(uv_dx) + length(uv_dy) <= 1e-6) {
        uv_dx = vec2(1.0, 0.0);
        uv_dy = vec2(0.0, 1.0);
    }

    vec3 t_ = (uv_dy.t * dFdx(pPosition) - uv_dx.t * dFdy(pPosition)) /
        (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);

    vec3 n, t, b, ng;

   
    if ((material & (1 << 4)) > 0) {
      ng = normalize(pNormal);
      t = normalize(t_ - ng * dot(ng, t_));
      b = cross(ng, t);
    } else {
       ng = normalize(cross(dFdx(pPosition), dFdy(pPosition)));
       t = normalize(t_ - ng * dot(ng, t_));
       b = cross(ng, t);
    }

    // For a back-facing surface, the tangential basis vectors are negated.
    if (gl_FrontFacing == false) {
        t *= -1.0;
        b *= -1.0;
        ng *= -1.0;
    }

    return normalize(mat3(t, b, ng) * tangentNormal);
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
