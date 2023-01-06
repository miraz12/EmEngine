#include "GraphicsSystem.hpp"
#include "Window.hpp"

#include <iostream>

#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

GraphicsSystem::GraphicsSystem(ECSManager *ECSManager, Camera &cam)
    : System(ECSManager, ComponentTypeEnum::POSITION,
             ComponentTypeEnum::GRAPHICS),
      m_camera(cam) {
  initGL();
}

void GraphicsSystem::update(float /*dt*/) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  for (auto &e : m_entities) {
    PositionComponent *p = static_cast<PositionComponent *>(
        e->getComponent(ComponentTypeEnum::POSITION));
    GraphicsComponent *g = static_cast<GraphicsComponent *>(
        e->getComponent(ComponentTypeEnum::GRAPHICS));
    g->grapObj->draw(m_camera);
  }
}

void GraphicsSystem::initGL() {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glLineWidth(3.0f); // Sets line width of things like wireframe and draw lines
}
