#include "LightingComponent.hpp"

LightingComponent::LightingComponent(BaseLight *light, TYPE type) : m_t(type), light(light) {}

LightingComponent::~LightingComponent() { delete light; }
