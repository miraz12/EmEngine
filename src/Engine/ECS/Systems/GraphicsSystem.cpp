#include "GraphicsSystem.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

#include <iostream>

GraphicsSystem::GraphicsSystem(ECSManager *ECSManager)
    : System(ECSManager, ComponentTypeEnum::POSITION,
             ComponentTypeEnum::GRAPHICS)
{
  initGL();
}

void GraphicsSystem::update(float /*dt*/) {
  draw();

  for (auto &e : m_entities) {
    PositionComponent *p = static_cast<PositionComponent *>(
        e->getComponent(ComponentTypeEnum::POSITION));
    GraphicsComponent *g = static_cast<GraphicsComponent *>(
        e->getComponent(ComponentTypeEnum::GRAPHICS));
    g->grapObj->draw();
  }
}

void GraphicsSystem::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  m_simpleShaderProgram.use();
  m_camera.bindViewMatrix(m_simpleShaderProgram.getUniformLocation("viewMatrix"));

}

void GraphicsSystem::initGL() {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glLineWidth(3.0f); // Sets line width of things like wireframe and draw lines
  m_simpleShaderProgram.setupVertexAttributePointers();
}
