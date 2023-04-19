#version 300 es
precision highp float;

#define NR_POINT_LIGHTS 10

struct PointLight {
    vec3 color;
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    float radius;
};
struct DirectionalLight {
    vec3 color;
    vec3 direction;
    float ambientIntensity;
};

uniform sampler2D gPositionAo;
uniform sampler2D gNormalMetal;
uniform sampler2D gAlbedoSpecRough;
uniform sampler2D gEmissive;
uniform vec3 camPos;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int nrOfPointLights;

in vec2 texCoords;
out vec4 FragColor;

const float PI = 3.14159265359;

float DistributionGGX(vec3 normal, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(normal, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 normal, vec3 viewDir, vec3 L, float roughness) {
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotL = max(dot(normal, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 specularColor) {
    return specularColor + (1.0 - specularColor) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalcDirectionalLightPBR(DirectionalLight light, vec3 fragPos, vec3 viewDir, vec3 normal, float roughness, float metallic, vec3 specularColor) {
    // calculate per-light radiance
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(viewDir + L);
    float attenuation = light.ambientIntensity;
    vec3 radiance     = light.color * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(normal, H, roughness);
    float G   = GeometrySmith(normal, viewDir, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, viewDir), 0.0), specularColor);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = max(4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0), 0.00001);
    vec3 specular     = numerator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(normal, L), 0.0);
    return (kD / PI + specular) * radiance * NdotL;
}

vec3 CalcPointLightPBR(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal, float roughness, float metallic, vec3 specularColor) {
    // calculate per-light radiance
    vec3 L = normalize(light.position - fragPos);
    vec3 H = normalize(viewDir + L);
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance     = light.color * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(normal, H, roughness);
    float G   = GeometrySmith(normal, viewDir, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, viewDir), 0.0), specularColor);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = max(4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0), 0.0001);
    vec3 specular     = numerator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(normal, L), 0.0);
    return (kD / PI + specular) * radiance * NdotL;
}


void main() {
    vec3 fragPos = texture(gPositionAo, texCoords).rgb;
    vec3 normal = texture(gNormalMetal, texCoords).rgb;
    vec3 albedo = texture(gAlbedoSpecRough, texCoords).rgb;
    vec4 emissive = texture(gEmissive, texCoords);
    float ao = texture(gPositionAo, texCoords).a;
    float metallic = texture(gNormalMetal, texCoords).a;
    float roughness = texture(gAlbedoSpecRough, texCoords).a;

    normal = normalize(normal);
    vec3 viewDir = normalize(camPos - fragPos);

    vec3 specularColor = mix(vec3(0.04), albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    Lo +=  CalcDirectionalLightPBR(directionalLight, fragPos, viewDir, normal, roughness, metallic, specularColor);

    for (int i = 0; i < nrOfPointLights; i++) {
        // calculate distance between light source and current fragment
        float distance = length(pointLights[i].position - fragPos);
        if(distance < pointLights[i].radius) {
            Lo +=  CalcPointLightPBR(pointLights[i], fragPos, viewDir, normal, roughness, metallic, specularColor);
        }
    }

    vec3 color = Lo * ao * albedo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(vec3(color), 1.0) + emissive;
}