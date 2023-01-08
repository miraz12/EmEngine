#include "LightingSystem.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"

LightingSystem::LightingSystem(ECSManager *ECSManager) : System(ECSManager, ComponentTypeEnum::LIGHTING, ComponentTypeEnum::POSITION) {
    for (auto &e : m_entities) {
        PositionComponent *p = static_cast<PositionComponent *>(
            e->getComponent(ComponentTypeEnum::POSITION));
        GraphicsComponent *g = static_cast<GraphicsComponent *>(
            e->getComponent(ComponentTypeEnum::GRAPHICS));
    }
}
