#version 300 es
precision highp float;

#define NR_POINT_LIGHTS 10

struct BaseLight {
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
};

struct PointLight {
    BaseLight base;
    vec3 position;
    float constant;
    float linear;
    float quadratic;
};
struct DirectionalLight {
    BaseLight base;
    vec3 direction;
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform vec3 camPos;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int nrOfPointLights;

in vec2 texCoords;
out vec4 FragColor;

vec4 CalcLightInternal(BaseLight light, vec3 normal, vec3 direction, vec3 viewDir, float shininess ) {
    vec4 ambientColor = vec4(light.color * light.ambientIntensity, 1.0f);

    vec4 diffuseColor  = vec4(0, 0, 0, 0);
    vec4 specularColor = vec4(0, 0, 0, 0);

    vec3 lightDir = normalize(-direction); //light direction from the fragment position
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    diffuseColor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0f);

    vec3 lightReflect = normalize(reflect(-lightDir, normal));
    float specularFactor = pow(max(dot(viewDir, lightReflect), 0.0), shininess);
    specularColor = vec4(light.color * 1.0 * specularFactor, 1.0f);
    return (ambientColor + diffuseColor + specularColor);
}   

vec4 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, float shininess ) {
    return CalcLightInternal(light.base, normal, light.direction, viewDir, shininess);
}
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos, float shininess) {
    vec3 direction = fragPos - light.position;
    float dist = length(direction);
    direction = normalize(direction);

    vec4 color = CalcLightInternal(light.base, normal, direction, viewDir, shininess);
    float attenuation = light.constant +
                        light.linear * dist +
                        light.quadratic * dist * dist;

    return color / attenuation;
}


void main() {
    vec3 fragPos = texture(gPosition, texCoords).rgb;
    vec3 normal = texture(gNormal, texCoords).rgb;
    vec3 diffuse = texture(gAlbedoSpec, texCoords).rgb;
    float specular = texture(gAlbedoSpec, texCoords).a;
    float shininess = 52.0;

    vec3 viewDir = normalize(camPos - fragPos); //Direction vector from fragment to camera

    vec4 color = vec4(0.0);
    color +=  CalcDirectionalLight(directionalLight, normal, viewDir, shininess);

    for (int i = 0; i < nrOfPointLights; i++) {
        color += CalcPointLight(pointLights[i], normal, viewDir, fragPos, shininess);
    }


    FragColor =  vec4(diffuse, specular) * color;
}
