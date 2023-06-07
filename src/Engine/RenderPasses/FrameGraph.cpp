#include "FrameGraph.hpp"
#include "Managers/FrameBufferManager.hpp"
#include <Camera.hpp>
#include <RenderPasses/CubeMapPass.hpp>
#include <RenderPasses/DebugPass.hpp>
#include <RenderPasses/GeometryPass.hpp>
#include <RenderPasses/LightPass.hpp>
#include <RenderPasses/ShadowPass.hpp>

// #define _DEBUG_

FrameGraph::FrameGraph() {
  // glPointSize(5.f);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glLineWidth(20.0f); // Sets line width of things like wireframe and draw lines
  glColorMask(true, true, true, true);
  // glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

  setViewport(m_width, m_height);

  m_renderPass.push_back(new ShadowPass());
  m_renderPass.push_back(new GeometryPass());
  m_renderPass.push_back(new LightPass());
  m_renderPass.push_back(new CubeMapPass());
#ifdef _DEBUG_
  m_renderPass.push_back(new DebugPass());
#endif
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
