#include "FrameGraph.hpp"
#include "Managers/FrameBufferManager.hpp"
#include <ECS/Systems/PhysicsSystem.hpp>
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
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glLineWidth(0.5f); // Sets line width of things like wireframe and draw lines
  glColorMask(true, true, true, true);

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

#ifndef EMSCRIPTEN
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
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glViewport(0, 0, m_width, m_height);

  // Execute all render passes except debug pass first
  for (size_t i = 0; i < m_renderPass.size(); i++) {

#ifndef EMSCRIPTEN
    if (i != static_cast<size_t>(PassId::kDebug)) {
#endif
      m_renderPass[i]->Execute(eManager);
#ifndef EMSCRIPTEN
    }
#endif
  }

#ifndef EMSCRIPTEN
  //  Execute debug pass last to ensure it's drawn on top
  auto debugPassIndex = static_cast<size_t>(PassId::kDebug);
  if (debugPassIndex < m_renderPass.size()) {
    m_renderPass[debugPassIndex]->Execute(eManager);
  }
#endif
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
