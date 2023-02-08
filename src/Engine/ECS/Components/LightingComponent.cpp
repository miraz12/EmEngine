#include "LightingComponent.hpp"

LightingComponent::LightingComponent() { m_componentType = ComponentTypeEnum::LIGHTING; }

LightingComponent::~LightingComponent() { delete light; }

void LightingComponent::SetupPointLight(glm::vec3 color, float constant, float linear,
                                        float quadratic, glm::vec3 pos) {
  m_t = TYPE::POINT;
  PointLight *pLight = new PointLight();
  pLight->position = pos;
  pLight->color = color;
  pLight->constant = constant;
  pLight->linear = linear;
  pLight->quadratic = quadratic;
  light = pLight;
}
void LightingComponent::SetupDirectionalLight(glm::vec3 color, float ambient, glm::vec3 dir) {
  m_t = TYPE::DIRECTIONAL;
  DirectionalLight *dLight = new DirectionalLight();
  dLight->direction = dir;
  dLight->color = color;
  dLight->ambientIntensity = ambient;
  light = dLight;
}
