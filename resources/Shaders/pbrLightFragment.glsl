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

uniform int debugView;
uniform sampler2D depthMap; // shadow map
uniform sampler2D gPositionAo;
uniform sampler2D gNormalMetal;
uniform sampler2D gAlbedoSpecRough;
uniform sampler2D gEmissive;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 camPos;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int nrOfPointLights;
uniform mat4 lightSpaceMatrix;

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

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
   // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(depthMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    ivec2 textureSize = textureSize(depthMap, 0);
    vec2 texelSize = vec2(1.0 / float(textureSize.x), 1.0 / float(textureSize.y));
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

vec3 CalcDirectionalLightPBR(DirectionalLight light, vec3 fragPos, vec3 viewDir, vec3 normal, float roughness, float metallic, vec3 specularColor, vec4 lightPos, vec3 albedo) {
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
    return (kD * albedo / PI + specular) * radiance * NdotL; 
}

vec3 CalcPointLightPBR(PointLight light, vec3 fragPos, vec3 viewDir, vec3 normal, float roughness, float metallic, vec3 specularColor, vec3 albedo) {

//---
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
    return (kD * albedo / PI + specular) * radiance * NdotL; 
}


void main() {
    vec3 fragPos = texture(gPositionAo, texCoords).rgb;
    vec3 normal = texture(gNormalMetal, texCoords).rgb;
    // vec3 albedo = texture(gAlbedoSpecRough, texCoords).rgb;
    vec3 albedo     = pow(texture(gAlbedoSpecRough, texCoords).rgb, vec3(2.2));
    vec3 emissive = texture(gEmissive, texCoords).rgb;
    float ao = texture(gPositionAo, texCoords).a;
    float metallic = texture(gNormalMetal, texCoords).a;
    float roughness = texture(gAlbedoSpecRough, texCoords).a;
    vec4 lightPos = lightSpaceMatrix * vec4(texture(gPositionAo, texCoords).rgb, 1.0);

    vec3 viewDir = normalize(camPos - fragPos);
    vec3 reflection = reflect(-viewDir, normal); 

    vec3 specularColor = mix(vec3(0.04), albedo, metallic);

    // reflectance equation
    vec3 Lo = CalcDirectionalLightPBR(directionalLight, fragPos, viewDir, normal, roughness, metallic, specularColor, lightPos, albedo);

    for (int i = 0; i < nrOfPointLights; i++) {
        // calculate distance between light source and current fragment
        float distance = length(pointLights[i].position - fragPos);
        if(distance < pointLights[i].radius) {
            Lo +=  CalcPointLightPBR(pointLights[i], fragPos, viewDir, normal, roughness, metallic, specularColor, albedo);
        }
    }
    
    // ambient lighting (we now use IBL as the ambient term)
    vec3 F = fresnelSchlickRoughness(max(dot(normal, viewDir), 0.0), specularColor, roughness); 
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  

    vec3 irradiance = texture(irradianceMap, normal).rgb;
    vec3 diffuse      = irradiance * albedo;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, reflection,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    //TODO: Does shadows and directional lights work like this in PBR??
    float shadows = ShadowCalculation(lightPos, normal, directionalLight.direction);
    vec3 ambient = (kD * diffuse + specular) * (1.0 - shadows)  * ao;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    if (debugView == 0) {
        FragColor = vec4(vec3(color) + emissive, 1.0) ;
    } else if (debugView == 1) {
        FragColor = vec4(vec3(albedo), 1.0) ;
    } else if (debugView == 2) {
        FragColor = vec4(vec3(normal), 1.0);
    } else if (debugView == 3) {
        FragColor = vec4(vec3(ao), 1.0);
    } else if (debugView == 4) {
        FragColor = vec4(vec3(emissive), 1.0);
    } else if (debugView == 5) {
        FragColor = vec4(vec3(metallic), 1.0);
    } else if (debugView == 6) {
        FragColor = vec4(vec3(roughness), 1.0);
    }
}
