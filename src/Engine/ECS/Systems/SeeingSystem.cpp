#include "SeeingSystem.hpp"
#include "ECS/Components/CollisionComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "ECS/Components/SeeingComponent.hpp"
#include "Physics/SAT.hpp"

#include <glm/gtx/rotate_vector.hpp>
#include <iostream>

SeeingSystem::SeeingSystem(ECSManager *ECSManager)
    : System(ECSManager, ComponentTypeEnum::COLLISION,
             ComponentTypeEnum::POSITION) {}

void SeeingSystem::update(float /*dt*/) {}
