#include "GraphicsSystem.hpp"
#include "System.hpp"

#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <RenderPasses/FrameGraph.hpp>

GraphicsSystem::GraphicsSystem()
  : m_fGraph(std::make_unique<FrameGraph>())
{
}

GraphicsSystem::~GraphicsSystem() = default;

void
GraphicsSystem::update(float /*dt*/)
{
  if (m_manager->getRenderGraphics()) {
    m_fGraph->draw(*m_manager);
  }
}

void
GraphicsSystem::setViewport(u32 w, u32 h)
{
  m_fGraph->setViewport(w, h);
}
