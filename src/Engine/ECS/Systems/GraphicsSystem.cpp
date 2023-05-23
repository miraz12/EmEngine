#include "GraphicsSystem.hpp"

#include <ECS/Systems/System.hpp>
#include <RenderPasses/FrameGraph.hpp>
#include <iostream>
#include <vector>

#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#include "ECS/ECSManager.hpp"

GraphicsSystem::GraphicsSystem(ECSManager *ECSManager, Camera &cam)
    : System(ECSManager), m_camera(cam), m_fboManager(FrameBufferManager::getInstance()),
      m_fGraph(*new FrameGraph()) {}

void GraphicsSystem::update(float /*dt*/) { m_fGraph.draw(*m_manager); }

void GraphicsSystem::setViewport(unsigned int w, unsigned int h) { m_fGraph.setViewport(w, h); }
