#include "LightingSystem.hpp"

#include <glm/gtx/string_cast.hpp>
#include <iostream>

#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "Managers/FrameBufferManager.hpp"
#include "Types/LightTypes.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

LightingSystem::LightingSystem(ECSManager *ECSManager, Camera &cam)
    : System(ECSManager, ComponentTypeEnum::LIGHTING), m_camera(cam) {
  initGL();
}
void LightingSystem::update(float /* dt */) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_shaderProgram.use();

  int numPLights = 0;
  for (auto &e : m_entities) {
    LightingComponent *g =
        static_cast<LightingComponent *>(e->getComponent(ComponentTypeEnum::LIGHTING));

    LightingComponent::TYPE t = g->getType();
    if (t == LightingComponent::DIRECTIONAL) {
      DirectionalLight *light = static_cast<DirectionalLight *>(g->getBaseLight());
      glUniform3fv(m_shaderProgram.getUniformLocation("directionalLight.direction"), 1,
                   glm::value_ptr(light->direction));

      glUniform3fv(m_shaderProgram.getUniformLocation("directionalLight.base.color"), 1,
                   glm::value_ptr(light->color));

      glUniform1f(m_shaderProgram.getUniformLocation("directionalLight.base.ambientIntensity"),
                  light->ambientIntensity);

      glUniform1f(m_shaderProgram.getUniformLocation("directionalLight.base.diffuseIntensity"),
                  light->diffuseIntensity);

    } else if (t == LightingComponent::POINT) {
      PointLight *light = static_cast<PointLight *>(g->getBaseLight());
      glUniform3fv(m_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                      "].position"),
                   1, glm::value_ptr(light->position));

      glUniform3fv(m_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                      "].base.color"),
                   1, glm::value_ptr(light->color));

      glUniform1f(m_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                     "].base.ambientIntensity"),
                  light->ambientIntensity);

      glUniform1f(m_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                     "].base.diffuseIntensity"),
                  light->diffuseIntensity);

      glUniform1f(m_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                     "].constant"),
                  light->constant);

      glUniform1f(m_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                     "].linear"),
                  light->linear);

      glUniform1f(m_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                     "].quadratic"),
                  light->quadratic);
      numPLights++;
    }
  }

  glUniform1i(m_shaderProgram.getUniformLocation("nrOfPointLights"), numPLights);

  glUniform3fv(m_shaderProgram.getUniformLocation("camPos"), 1,
               glm::value_ptr(m_camera.getPosition()));

  glBindVertexArray(quadVAO);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, gAlbedo);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void LightingSystem::initGL() {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glLineWidth(3.0f); // Sets line width of things like wireframe and draw lines
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glColorMask(true, true, true, true);

  glGenFramebuffers(1, &gBuffer);
  glGenTextures(1, &gPosition);
  glGenTextures(1, &gNormal);
  glGenTextures(1, &gAlbedo);
  glGenRenderbuffers(1, &rboDepth);
  FrameBufferManager::getInstance().setFBO("gBuffer", gBuffer);

  setViewport(m_width, m_height);

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

  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
}

void LightingSystem::setViewport(unsigned int w, unsigned int h) {
  m_width = w;
  m_height = h;
  m_camera.setSize(w, h);

  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

  // - position color buffer
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

  // - normal color buffer
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

  // - color
  glBindTexture(GL_TEXTURE_2D, gAlbedo);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

  // - tell OpenGL which color attachments we'll use (of this framebuffer) for
  // rendering
  unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};

  glDrawBuffers(3, attachments);
  // create and attach depth buffer (renderbuffer)
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "Framebuffer not complete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
