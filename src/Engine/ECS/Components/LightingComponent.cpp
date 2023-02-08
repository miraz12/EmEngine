#include "LightingComponent.hpp"

LightingComponent::LightingComponent(BaseLight *light, TYPE type) : m_t(type), light(light) {
  m_componentType = ComponentTypeEnum::LIGHTING;
}

LightingComponent::~LightingComponent() { delete light; }
