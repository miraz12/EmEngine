#include "FrameGraph.hpp"
#include <ECS/Systems/PhysicsSystem.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <RenderPasses/BloomPass.hpp>
#include <RenderPasses/CubeMapPass.hpp>
#include <RenderPasses/DebugPass.hpp>
#include <RenderPasses/FxaaPass.hpp>
#include <RenderPasses/GeometryPass.hpp>
#include <RenderPasses/LightPass.hpp>
#include <RenderPasses/ParticlePass.hpp>
#include <RenderPasses/ShadowPass.hpp>

FrameGraph::FrameGraph()
{
  auto& device = gfx::GraphicsDevice::getInstance();
  device.clearColor(std::nullopt, glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
  device.setLineWidth(
    0.5f); // Sets line width of things like wireframe and draw lines
  device.setColorMask(true, true, true, true);

  // This is not what controls render order, check PassId in header instead.
  m_renderPass[static_cast<size_t>(PassId::kShadow)] =
    std::make_unique<ShadowPass>();
  m_renderPass[static_cast<size_t>(PassId::kGeom)] =
    std::make_unique<GeometryPass>();
  m_renderPass[static_cast<size_t>(PassId::kLight)] =
    std::make_unique<LightPass>();
  m_renderPass[static_cast<size_t>(PassId::kParticle)] =
    std::make_unique<ParticlePass>();
  m_renderPass[static_cast<size_t>(PassId::kCube)] =
    std::make_unique<CubeMapPass>();
  m_renderPass[static_cast<size_t>(PassId::kBloom)] =
    std::make_unique<BloomPass>();
  m_renderPass[static_cast<size_t>(PassId::kFxaa)] =
    std::make_unique<FxaaPass>();

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  m_renderPass[static_cast<size_t>(PassId::kDebug)] =
    std::make_unique<DebugPass>();
#endif

  for (auto& p : m_renderPass) {
    p->Init(*this);
  }
  setViewport(m_width, m_height);
}

void
FrameGraph::draw(ECSManager& eManager)
{
  auto& resources = gfx::RenderResources::getInstance();
  resources.clearColor(std::nullopt, 0.0f, 0.0f, 0.0f, 0.0f);
  resources.clearDepth(1.0f);
  resources.clearStencil(0);
  resources.setViewportRect(0, 0, m_width, m_height);

  // Execute all render passes except debug pass first
  for (const auto& renderPas : m_renderPass) {
    renderPas->Execute(eManager);
  }
}

void
FrameGraph::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;
  for (const auto& pass : m_renderPass) {
    pass->setViewport(w, h);
  }
}
