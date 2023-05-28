#include "LightingComponent.hpp"

LightingComponent::LightingComponent(std::shared_ptr<BaseLight> light, TYPE type)
    : m_t(type), light(light) {}
