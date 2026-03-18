#version 300 es
precision highp float;
precision highp sampler2DArrayShadow;

#define NR_POINT_LIGHTS 10
#define NUM_CASCADES 4

struct PointLight
{
  vec3 color;
  vec3 position;
  float constant;
  float linear;
  float quadratic;
  float radius;
};
struct DirectionalLight
{
  vec3 color;
  vec3 direction;
  float intensity;
};

uniform int debugView;
uniform sampler2DArrayShadow depthMapArray; // CSM shadow map array
uniform sampler2D gPositionAo;
uniform sampler2D gNormalMetal;
uniform sampler2D gAlbedoSpecRough;
uniform sampler2D gEmissive;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform vec3 camPos;
uniform mat4 viewMatrix;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int nrOfPointLights;

// Cascade data from UBO (binding point 0)
layout(std140) uniform CascadeData
{
  mat4 lightSpaceMatrices[NUM_CASCADES];
  vec4 cascadeSplits; // x, y, z used for 4 cascades
  ivec4 config;       // x: numCascades, y: shadowMapSize
};

in vec2 texCoords;
out vec4 FragColor;

const float PI = 3.14159265359;

float
DistributionGGX(vec3 normal, vec3 H, float roughness)
{
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(normal, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float nom = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return nom / denom;
}

float
GeometrySchlickGGX(float NdotV, float roughness)
{
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;

  float nom = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}

float
GeometrySmith(vec3 normal, vec3 viewDir, vec3 L, float roughness)
{
  float NdotV = max(dot(normal, viewDir), 0.0);
  float NdotL = max(dot(normal, L), 0.0);
  float ggx2 = GeometrySchlickGGX(NdotV, roughness);
  float ggx1 = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}

vec3
fresnelSchlick(float cosTheta, vec3 specularColor)
{
  return specularColor + (1.0 - specularColor) * pow(1.0 - cosTheta, 5.0);
}

vec3
fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) *
                pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Sample shadow from a specific cascade
float
SampleCascadeShadow(vec3 fragPos,
                    vec3 normal,
                    vec3 lightDir,
                    int cascadeIndex)
{
  // Transform to light space for selected cascade
  vec4 fragPosLightSpace =
    lightSpaceMatrices[cascadeIndex] * vec4(fragPos, 1.0);
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  projCoords = projCoords * 0.5 + 0.5;

  // Out of bounds check - return 1.0 (fully lit) if outside shadow map coverage
  if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 ||
      projCoords.y > 1.0 || projCoords.z > 1.0) {
    return 1.0;
  }

  // Adaptive bias
  float bias = max(0.005 * (1.0 - dot(normal, -lightDir)), 0.002);
  float currentDepth = projCoords.z - bias;

  // PCF with hardware shadow sampling
  float shadow = 0.0;
  vec2 texelSize = 1.0 / vec2(float(config.y)); // shadowMapSize

  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      vec2 offset = vec2(x, y) * texelSize;
      // Hardware PCF via shadow2DArray (compare happens in hardware)
      shadow += texture(depthMapArray,
                        vec4(projCoords.xy + offset,
                             float(cascadeIndex),
                             currentDepth));
    }
  }
  shadow /= 9.0;

  return shadow;
}

float
ShadowCalculation(vec3 fragPos, vec3 normal, vec3 lightDir)
{
  // Calculate view-space depth for cascade selection
  vec4 viewPos = viewMatrix * vec4(fragPos, 1.0);
  float viewDepth = abs(viewPos.z);

  // Select cascade based on view depth
  int cascadeIndex = 0;
  if (viewDepth > cascadeSplits.z) {
    cascadeIndex = 3;
  } else if (viewDepth > cascadeSplits.y) {
    cascadeIndex = 2;
  } else if (viewDepth > cascadeSplits.x) {
    cascadeIndex = 1;
  }

  // Sample shadow from selected cascade
  float shadow = SampleCascadeShadow(fragPos, normal, lightDir, cascadeIndex);

  // Cascade blending (10% overlap region)
  if (cascadeIndex < 3) {
    float currentSplit = (cascadeIndex == 0)   ? 0.0
                         : (cascadeIndex == 1) ? cascadeSplits.x
                         : (cascadeIndex == 2) ? cascadeSplits.y
                                               : cascadeSplits.z;
    float nextSplit = (cascadeIndex == 0)   ? cascadeSplits.x
                      : (cascadeIndex == 1) ? cascadeSplits.y
                      : (cascadeIndex == 2) ? cascadeSplits.z
                                            : cascadeSplits.w;
    float blendRange = (nextSplit - currentSplit) * 0.1; // 10% blend zone
    float blendFactor = smoothstep(nextSplit - blendRange, nextSplit, viewDepth);

    if (blendFactor > 0.001) {
      float shadowNext =
        SampleCascadeShadow(fragPos, normal, lightDir, cascadeIndex + 1);
      shadow = mix(shadow, shadowNext, blendFactor);
    }
  }

  return shadow;
}

vec3
CalcDirectionalLightPBR(DirectionalLight light,
                        vec3 fragPos,
                        vec3 viewDir,
                        vec3 normal,
                        float roughness,
                        float metallic,
                        vec3 specularColor,
                        vec3 albedo)
{
  // calculate per-light radiance
  vec3 L = normalize(-light.direction);
  vec3 H = normalize(viewDir + L);
  vec3 radiance = light.color * light.intensity;

  // cook-torrance brdf
  float NDF = DistributionGGX(normal, H, roughness);
  float G = GeometrySmith(normal, viewDir, L, roughness);
  vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), specularColor);

  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metallic;

  vec3 numerator = NDF * G * F;
  float denominator = max(
    4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0), 0.0001);
  vec3 specular = numerator / denominator;

  // Calculate shadow with CSM
  float NdotL = max(dot(normal, L), 0.0);
  float shadowFactor = ShadowCalculation(fragPos, normal, light.direction);

  return (kD * albedo / PI + specular) * radiance * NdotL * shadowFactor;
}

vec3
CalcPointLightPBR(PointLight light,
                  vec3 fragPos,
                  vec3 viewDir,
                  vec3 normal,
                  float roughness,
                  float metallic,
                  vec3 specularColor,
                  vec3 albedo)
{

  // calculate per-light radiance
  vec3 L = normalize(light.position - fragPos);
  vec3 H = normalize(viewDir + L);
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (distance * distance);
  vec3 radiance = light.color * attenuation;

  // cook-torrance brdf
  float NDF = DistributionGGX(normal, H, roughness);
  float G = GeometrySmith(normal, viewDir, L, roughness);
  vec3 F = fresnelSchlick(max(dot(H, viewDir), 0.0), specularColor);

  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metallic;

  vec3 numerator = NDF * G * F;
  float denominator = max(
    4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, L), 0.0), 0.0001);
  vec3 specular = numerator / denominator;

  // add to outgoing radiance Lo
  float NdotL = max(dot(normal, L), 0.0);
  return (kD * albedo / PI + specular) * radiance * NdotL;
}

void
main()
{
  vec3 fragPos = texture(gPositionAo, texCoords).rgb;
  vec3 normal = texture(gNormalMetal, texCoords).rgb;
  // vec3 albedo = texture(gAlbedoSpecRough, texCoords).rgb;
  vec3 albedo = pow(texture(gAlbedoSpecRough, texCoords).rgb, vec3(2.2));
  vec3 emissive = texture(gEmissive, texCoords).rgb;
  float ao = texture(gPositionAo, texCoords).a;
  float metallic = texture(gNormalMetal, texCoords).a;
  float roughness = texture(gAlbedoSpecRough, texCoords).a;

  vec3 viewDir = normalize(camPos - fragPos);
  vec3 reflection = reflect(-viewDir, normal);

  vec3 specularColor = mix(vec3(0.04), albedo, metallic);

  // reflectance equation
  vec3 Lo = CalcDirectionalLightPBR(directionalLight,
                                    fragPos,
                                    viewDir,
                                    normal,
                                    roughness,
                                    metallic,
                                    specularColor,
                                    albedo);

  for (int i = 0; i < nrOfPointLights; i++) {
    // calculate distance between light source and current fragment
    float distance = length(pointLights[i].position - fragPos);
    if (distance < pointLights[i].radius) {
      Lo += CalcPointLightPBR(pointLights[i],
                              fragPos,
                              viewDir,
                              normal,
                              roughness,
                              metallic,
                              specularColor,
                              albedo);
    }
  }

  // ambient lighting (we now use IBL as the ambient term)
  vec3 F = fresnelSchlickRoughness(
    max(dot(normal, viewDir), 0.0), specularColor, roughness);
  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;

  vec3 irradiance = texture(irradianceMap, normal).rgb;
  vec3 diffuse = irradiance * albedo;

  // sample both the pre-filter map and the BRDF lut and combine them together
  // as per the Split-Sum approximation to get the IBL specular part.
  const float MAX_REFLECTION_LOD = 4.0;
  vec3 prefilteredColor =
    textureLod(prefilterMap, reflection, roughness * MAX_REFLECTION_LOD).rgb;
  vec2 brdf =
    texture(brdfLUT, vec2(max(dot(normal, viewDir), 0.0), roughness)).rg;
  vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

  vec3 ambient = (kD * diffuse + specular) * ao;

  vec3 color = ambient + Lo;

  if (debugView == 0) {
    FragColor = vec4(vec3(color) + emissive, 1.0);
  } else if (debugView == 1) {
    FragColor = vec4(vec3(albedo), 1.0);
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
  } else if (debugView == 7) {
    // Cascade visualization
    vec4 viewPos = viewMatrix * vec4(fragPos, 1.0);
    float viewDepth = abs(viewPos.z);

    vec3 cascadeColor = vec3(1.0, 0.0, 0.0); // Red = cascade 0
    if (viewDepth > cascadeSplits.z) {
      cascadeColor = vec3(0.0, 1.0, 1.0); // Cyan = cascade 3
    } else if (viewDepth > cascadeSplits.y) {
      cascadeColor = vec3(0.0, 0.0, 1.0); // Blue = cascade 2
    } else if (viewDepth > cascadeSplits.x) {
      cascadeColor = vec3(0.0, 1.0, 0.0); // Green = cascade 1
    }

    FragColor = vec4(cascadeColor, 1.0);
  }
}
