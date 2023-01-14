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
    : System(ECSManager, ComponentTypeEnum::POSITION,
             ComponentTypeEnum::GRAPHICS),
      m_camera(cam) {
  initGL();
}

void GraphicsSystem::update(float /*dt*/) {
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // Draw geometry
  for (auto &e : m_entities) {
    PositionComponent *p = static_cast<PositionComponent *>(
        e->getComponent(ComponentTypeEnum::POSITION));
    GraphicsComponent *g = static_cast<GraphicsComponent *>(
        e->getComponent(ComponentTypeEnum::GRAPHICS));
    g->grapObj->draw(m_camera);
  }

  // Draw lights
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_shaderProgram.use();
  glBindVertexArray(quadVAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, gAlbedo);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

void GraphicsSystem::initGL() {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glLineWidth(3.0f); // Sets line width of things like wireframe and draw lines

  glBindVertexArray(0);
  glGenFramebuffers(1, &gBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

  // - position color buffer
  glGenTextures(1, &gPosition);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA,
               GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         gPosition, 0);

  // - normal color buffer
  glGenTextures(1, &gNormal);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA,
               GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         gNormal, 0);

  // - color
  glGenTextures(1, &gAlbedo);
  glBindTexture(GL_TEXTURE_2D, gAlbedo);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         gAlbedo, 0);

  // - tell OpenGL which color attachments we'll use (of this framebuffer) for
  // rendering
  unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                 GL_COLOR_ATTACHMENT2};

  glDrawBuffers(3, attachments);
  // create and attach depth buffer (renderbuffer)
  unsigned int rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width,
                        m_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rboDepth);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glUniform1i(m_shaderProgram.getUniformLocation("gPosition"), 0);
  glUniform1i(m_shaderProgram.getUniformLocation("gNormal"), 1);
  glUniform1i(m_shaderProgram.getUniformLocation("gAlbedoSpec"), 2);

  unsigned int quadVBO;
  float quadVertices[] = {
      // positions        // texture Coords
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
  };
  // setup plane VAO
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);

  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, gAlbedo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glBindVertexArray(0);
}

void GraphicsSystem::setViewport(unsigned int w, unsigned int h) {
  m_width = w;
  m_height = h;
  m_camera.setSize(w, h);

  glBindVertexArray(0);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

  // - position color buffer
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB,
               GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         gPosition, 0);

  // - normal color buffer
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB,
               GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         gNormal, 0);

  // - color + specular color buffer
  glBindTexture(GL_TEXTURE_2D, gAlbedo);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         gAlbedo, 0);

  // - tell OpenGL which color attachments we'll use (of this framebuffer) for
  // rendering
  unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
                                 GL_COLOR_ATTACHMENT2};
  glDrawBuffers(3, attachments);

  // create and attach depth buffer (renderbuffer)
  unsigned int rboDepth;
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width,
                        m_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rboDepth);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
