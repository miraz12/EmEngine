#version 300 es
precision highp float;

#define NR_POINT_LIGHTS 10
struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
};
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
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
    vec3 ambientColor = diffuse * light.ambientIntensity; //Ambient lighting
    vec3 diffuseColor = light.color * diff * diffuse;
    vec3 specularColor = light.color * spec * specular;
    return (ambientColor + diffuseColor + specularColor);
}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuse, float specular, float shininess) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambientColor = diffuse; //Ambient lighting
    vec3 diffuseColor = light.color * diff * diffuse;
    vec3 specularColor = light.color * spec * specular;
    ambientColor *= attenuation;
    diffuseColor *= attenuation;
    specularColor *= attenuation;
    return (ambientColor + diffuseColor + specularColor);
}


void main() {
    vec3 fragPos = texture(gPosition, texCoords).rgb;
    vec3 normal = texture(gNormal, texCoords).rgb;
    vec3 diffuse = texture(gAlbedoSpec, texCoords).rgb;
    float specular = texture(gAlbedoSpec, texCoords).a;
    float shininess = 32.0;

    vec3 viewDir = normalize(camPos - fragPos); //Direction vector from fragment to camera

    vec3 color =  CalcDirectionalLight(directionalLight, normal, viewDir, diffuse, specular, shininess);

	for (int i = 0; i < nrOfPointLights; i++) {
		color += CalcPointLight(pointLights[i], normal, fragPos, viewDir, diffuse, specular, shininess);
	}


    FragColor = vec4(color, 1.0);
}
