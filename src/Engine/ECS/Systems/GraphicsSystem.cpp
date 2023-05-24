#include "GraphicsSystem.hpp"

#include <ECS/Systems/System.hpp>
#include <RenderPasses/FrameGraph.hpp>
#include <iostream>
#include <vector>

#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"



#include "ECS/ECSManager.hpp"

GraphicsSystem::GraphicsSystem(ECSManager *ECSManager)
    : System(ECSManager), m_fboManager(FrameBufferManager::getInstance()),
      m_fGraph(*new FrameGraph()) {}

void GraphicsSystem::update(float /*dt*/) { m_fGraph.draw(*m_manager); }

void GraphicsSystem::setViewport(unsigned int w, unsigned int h) { m_fGraph.setViewport(w, h); }
