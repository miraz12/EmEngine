#include "GraphicsSystem.hpp"
#include "System.hpp"

#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <RenderPasses/FrameGraph.hpp>

GraphicsSystem::GraphicsSystem()
    : m_fboManager(FrameBufferManager::getInstance()),
      m_fGraph(*new FrameGraph()) {}

GraphicsSystem::~GraphicsSystem() { delete &m_fGraph; }

void GraphicsSystem::update(float /*dt*/) { m_fGraph.draw(*m_manager); }

void GraphicsSystem::setViewport(u32 w, u32 h) { m_fGraph.setViewport(w, h); }
