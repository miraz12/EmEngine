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

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform vec3 camPos;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int nrOfPointLights;

in vec2 texCoords;
out vec4 FragColor;

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuse, float specular, float shininess ) {
    vec3 lightDir = normalize(-light.direction); //light direction from the fragment position
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // Combine results
    vec3 ambient = diffuse * light.ambientIntensity; //Ambient lighting
    vec3 finalDiffuse = light.color * diff * diffuse;
    vec3 finalSpecular = light.color * spec * specular;

    vec3 lighting = (ambient + finalDiffuse + finalSpecular);
    return lighting;
}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 fragPos, vec3 diffuse, float specular, float shininess) {
    vec3 lighting;
    vec3 lightDir = normalize(light.position - fragPos); //light direction from the fragment position
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Combine results
    vec3 finalDiffuse = light.color * diff * diffuse;
    vec3 finalSpecular = light.color * spec * specular;
    finalDiffuse *= attenuation;
    finalSpecular *= attenuation;
    lighting = finalDiffuse + finalSpecular;
    return lighting;
}


void main() {
    vec3 fragPos = texture(gPosition, texCoords).rgb;
    vec3 normal = texture(gNormal, texCoords).rgb;
    vec3 diffuse = texture(gAlbedoSpec, texCoords).rgb;
    float specular = 0.0;//texture(gAlbedoSpec, texCoords).a;
    float shininess = 34.0;

    vec3 viewDir = normalize(camPos - fragPos); //Direction vector from fragment to camera

    vec3 color = vec3(0.0);
    color +=  CalcDirectionalLight(directionalLight, normal, viewDir, diffuse, specular, shininess);

    for (int i = 0; i < nrOfPointLights; i++) {
        // calculate distance between light source and current fragment
        float distance = length(pointLights[i].position - fragPos);
        if(distance < pointLights[i].radius) {
            color += CalcPointLight(pointLights[i], normal, viewDir, fragPos, diffuse, specular, shininess);
        }
    }


    FragColor = vec4(color, 1.0);
}
