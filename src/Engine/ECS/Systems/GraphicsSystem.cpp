#include "GraphicsSystem.hpp"

#include <iostream>

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

GraphicsSystem::GraphicsSystem(ECSManager *ECSManager, Camera &cam)
    : System(ECSManager, ComponentTypeEnum::POSITION, ComponentTypeEnum::GRAPHICS), m_camera(cam),
      m_fboManager(FrameBufferManager::getInstance()) {}

void GraphicsSystem::update(float /*dt*/) {
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboManager.getFBO("gBuffer"));
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // Draw geometry
  for (auto &e : m_entities) {
    PositionComponent *p =
        static_cast<PositionComponent *>(e->getComponent(ComponentTypeEnum::POSITION));
    glm::mat4 model = p->calculateMatrix();
    GraphicsComponent *g =
        static_cast<GraphicsComponent *>(e->getComponent(ComponentTypeEnum::GRAPHICS));
    g->grapObj->draw(m_camera, model);
  }
}
