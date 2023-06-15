#include "FrameGraph.hpp"
#include "Managers/FrameBufferManager.hpp"
#include <Camera.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>
#include <RenderPasses/CubeMapPass.hpp>
#include <RenderPasses/DebugPass.hpp>
#include <RenderPasses/GeometryPass.hpp>
#include <RenderPasses/LightPass.hpp>
#include <RenderPasses/ShadowPass.hpp>

FrameGraph::FrameGraph() {
  // glPointSize(5.f);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glLineWidth(2.0f); // Sets line width of things like wireframe and draw lines
  glColorMask(true, true, true, true);
  // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  // Damn this is ugly..
  m_renderPass[static_cast<size_t>(PassId::kShadow)] = new ShadowPass();
  m_renderPass[static_cast<size_t>(PassId::kGeom)] = new GeometryPass();
  m_renderPass[static_cast<size_t>(PassId::kLight)] = new LightPass();
  m_renderPass[static_cast<size_t>(PassId::kCube)] = new CubeMapPass();
  m_renderPass[static_cast<size_t>(PassId::kDebug)] = new DebugPass();

  for (auto &p : m_renderPass) {
    p->Init(*this);
  }

  setViewport(m_width, m_height);
}

FrameGraph::~FrameGraph() {
  for (auto &p : m_renderPass) {
    delete p;
  }
}

void FrameGraph::draw(ECSManager &eManager) {
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  for (auto pass : m_renderPass) {
    pass->Execute(eManager);
  }
}

void FrameGraph::setViewport(uint32_t w, uint32_t h) {
  for (auto pass : m_renderPass) {
    pass->setViewport(w, h);
  }
}
