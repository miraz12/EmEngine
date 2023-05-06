#include "FrameGraph.hpp"
#include "Managers/FrameBufferManager.hpp"
#include <Camera.hpp>
#include <RenderPasses/GeometryPass.hpp>
#include <RenderPasses/LightPass.hpp>

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

FrameGraph::FrameGraph(Camera &cam) : m_camera(cam) {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glLineWidth(3.0f); // Sets line width of things like wireframe and draw lines
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glColorMask(true, true, true, true);

  setViewport(m_width, m_height);

  m_renderPass.push_back(new GeometryPass());
  m_renderPass.push_back(new LightPass());
}

void FrameGraph::draw(ECSManager &eManager) {
  for (auto pass : m_renderPass) {
    pass->Execute(eManager);
  }
}

void FrameGraph::setViewport(unsigned int w, unsigned int h) {
  for (auto pass : m_renderPass) {
    pass->setViewport(w, h);
  }
}
