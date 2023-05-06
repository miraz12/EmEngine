#include "GraphicsSystem.hpp"

#include <ECS/Systems/System.hpp>
#include <iostream>
#include <vector>

#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "Window.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#include "ECS/ECSManager.hpp"

GraphicsSystem::GraphicsSystem(ECSManager *ECSManager, Camera &cam)
    : System(ECSManager), m_camera(cam), m_fboManager(FrameBufferManager::getInstance()) {}

void GraphicsSystem::update(float /*dt*/) {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboManager.getFBO("gBuffer"));
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  std::vector<Entity> view = m_manager->view<GraphicsComponent, PositionComponent>();
  for (auto e : view) {
    PositionComponent *p = m_manager->getComponent<PositionComponent>(e);
    glm::mat4 model = p->calculateMatrix();
    GraphicsComponent *g = m_manager->getComponent<GraphicsComponent>(e);
    g->grapObj->draw(m_camera, model);
  }
}
