#include "ShadowPass.hpp"
#include "glm/geometric.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#include "ECS/ECSManager.hpp"
#include "RenderPasses/RenderPass.hpp"

ShadowPass::ShadowPass()
    : RenderPass("resources/Shaders/shadowVertex.glsl", "resources/Shaders/shadowFragment.glsl") {

  p_shaderProgram.use();
  p_shaderProgram.setUniformBinding("modelMatrix");
  p_shaderProgram.setUniformBinding("lightSpaceMatrix");
  p_shaderProgram.setUniformBinding("meshMatrix");

  p_shaderProgram.setAttribBinding("POSITION");

  unsigned int depthMapFbo;
  glGenFramebuffers(1, &depthMapFbo);
  p_fboManager.setFBO("depthMapFbo", depthMapFbo);

  unsigned int depthMap;
  glGenTextures(1, &depthMap);
  p_textureManager.setTexture("depthMap", depthMap);

  setViewport(p_width, p_height);
}

void ShadowPass::Execute(ECSManager &eManager) {
  p_fboManager.bindFBO("depthMapFbo");
  glEnable(GL_DEPTH_TEST);
  glClear(GL_DEPTH_BUFFER_BIT);
  glCullFace(GL_FRONT);

  p_shaderProgram.use();

  // TODO Don't do this every frame
  glm::mat4 lightProjection, lightView;
  glm::mat4 lightSpaceMatrix;
  float shadowBox = 9.0f;
  lightProjection = glm::ortho(-shadowBox, shadowBox, -shadowBox, shadowBox, 1.0f, 30.0f);
  glm::vec3 lightInvDir = -glm::normalize(eManager.dDir) * 20.0f;
  lightView = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  lightSpaceMatrix = lightProjection * lightView;
  glUniformMatrix4fv(p_shaderProgram.getUniformLocation("lightSpaceMatrix"), 1, GL_FALSE,
                     glm::value_ptr(lightSpaceMatrix));

  std::vector<Entity> view = eManager.view<GraphicsComponent, PositionComponent>();
  for (auto e : view) {
    // PositionComponent *p = eManager.getComponent<PositionComponent>(e);
    // glm::mat4 model = p->calculateMatrix();
    GraphicsComponent *g = eManager.getComponent<GraphicsComponent>(e);
    g->m_grapObj.drawGeom(p_shaderProgram);
  }

  glCullFace(GL_BACK);
}

void ShadowPass::setViewport(unsigned int w, unsigned int h) {
  p_width = w;
  p_height = h;

  p_fboManager.bindFBO("depthMapFbo");
  unsigned int depthMap = p_textureManager.bindTexture("depthMap");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, p_width, p_height, 0, GL_DEPTH_COMPONENT,
               GL_UNSIGNED_INT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  GLuint buffer = GL_NONE; // Emscripten strangeness
  glDrawBuffers(1, &buffer);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
