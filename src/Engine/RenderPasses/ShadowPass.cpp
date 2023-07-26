#include "ShadowPass.hpp"
#include "glm/geometric.hpp"

#include "ECS/ECSManager.hpp"
#include "RenderPasses/RenderPass.hpp"
#include <RenderPasses/FrameGraph.hpp>

ShadowPass::ShadowPass()
    : RenderPass("resources/Shaders/shadowVertex.glsl", "resources/Shaders/shadowFragment.glsl") {

  p_shaderProgram.use();
  p_shaderProgram.setUniformBinding("modelMatrix");
  p_shaderProgram.setUniformBinding("lightSpaceMatrix");
  p_shaderProgram.setUniformBinding("meshMatrix");

  p_shaderProgram.setAttribBinding("POSITION");

  uint32_t depthMapFbo;
  glGenFramebuffers(1, &depthMapFbo);
  p_fboManager.setFBO("depthMapFbo", depthMapFbo);

  uint32_t depthMap;
  glGenTextures(1, &depthMap);
  p_textureManager.setTexture("depthMap", depthMap);

  p_fboManager.bindFBO("depthMapFbo");
  p_textureManager.bindTexture("depthMap");
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

  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("FB error, status: 0x%x\n", Status);
  }

  setViewport(p_width, p_height);
}

void ShadowPass::Init(FrameGraph &fGraph) {
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture("depthMap");
}

void ShadowPass::Execute(ECSManager &eManager) {
  p_fboManager.bindFBO("depthMapFbo");
  glEnable(GL_DEPTH_TEST);
  glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
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

  std::vector<Entity> view = eManager.view<GraphicsComponent>();
  for (auto &e : view) {
    std::shared_ptr<PositionComponent> p = eManager.getComponent<PositionComponent>(e);
    if (p) {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
                         glm::value_ptr(p->model));
    } else {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
                         glm::value_ptr(glm::identity<glm::mat4>()));
    }

    std::shared_ptr<GraphicsComponent> g = eManager.getComponent<GraphicsComponent>(e);
    g->m_grapObj.drawGeom(p_shaderProgram);
  }

  glCullFace(GL_BACK);
  m_dirty = false;
}

void ShadowPass::setViewport(uint32_t w, uint32_t h) {
  p_width = w;
  p_height = h;

  m_dirty = true;
  p_fboManager.bindFBO("depthMapFbo");
  p_textureManager.bindTexture("depthMap");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, p_width, p_height, 0, GL_DEPTH_COMPONENT,
               GL_UNSIGNED_INT, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
