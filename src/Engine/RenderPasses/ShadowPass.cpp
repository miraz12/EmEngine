#include "ShadowPass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "LightingUtil.hpp"

#include "ECS/ECSManager.hpp"
#include "RenderPasses/RenderPass.hpp"
#include <RenderPasses/FrameGraph.hpp>

ShadowPass::ShadowPass()
  : RenderPass("resources/Shaders/shadow.vert", "resources/Shaders/shadow.frag")
{

  p_shaderProgram.use();
  p_shaderProgram.setUniformBinding("modelMatrix");
  p_shaderProgram.setUniformBinding("lightSpaceMatrix");
  p_shaderProgram.setUniformBinding("meshMatrix");
  p_shaderProgram.setUniformBinding("is_skinned");
  p_shaderProgram.setUniformBinding("jointMats");

  p_shaderProgram.setAttribBinding("POSITION");
  p_shaderProgram.setAttribBinding("JOINTS_0");
  p_shaderProgram.setAttribBinding("WEIGHTS_0");

  u32 jointMats;
  glGenTextures(1, &jointMats);
  p_textureManager.setTexture("jointMats", jointMats);

  p_textureManager.bindTexture("jointMats");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  u32 depthMapFbo;
  glGenFramebuffers(1, &depthMapFbo);
  p_fboManager.setFBO("depthMapFbo", depthMapFbo);

  u32 depthMap;
  glGenTextures(1, &depthMap);
  p_textureManager.setTexture("depthMap", depthMap);

  p_fboManager.bindFBO("depthMapFbo");
  p_textureManager.bindTexture("depthMap");
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_DEPTH_COMPONENT24,
               p_width,
               p_height,
               0,
               GL_DEPTH_COMPONENT,
               GL_UNSIGNED_INT,
               0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
  GLuint buffer = GL_NONE; // Emscripten strangeness
  glDrawBuffers(1, &buffer);
  glReadBuffer(GL_NONE);

  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("FB error, status: 0x%x\n", Status);
  }

  setViewport(p_width, p_height);
}

void
ShadowPass::Init(FrameGraph& fGraph)
{
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture(
    "depthMap");
}

void
ShadowPass::Execute(ECSManager& eManager)
{
  p_fboManager.bindFBO("depthMapFbo");
  glViewport(0, 0, p_width, p_height);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDepthMask(GL_TRUE);
  glClearDepthf(1.0f);
  glClear(GL_DEPTH_BUFFER_BIT);
  // Use back-face culling (normal rendering) for shadow map
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  p_shaderProgram.use();

  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  std::vector<Entity> view = eManager.view<LightingComponent>();
  for (auto e : view) {
    std::shared_ptr<LightingComponent> g =
      eManager.getComponent<LightingComponent>(e);

    switch (g->getType()) {
      case LightingComponent::TYPE::DIRECTIONAL: {
        auto& light = static_cast<DirectionalLight&>(g->getBaseLight());
        glm::mat4 lightSpaceMatrix = LightingUtil::calculateLightSpaceMatrix(
          light.direction, cam->m_position);
        glUniformMatrix4fv(
          p_shaderProgram.getUniformLocation("lightSpaceMatrix"),
          1,
          GL_FALSE,
          glm::value_ptr(lightSpaceMatrix));
        break;
      }
      default:
        break;
    }
  }

  std::vector<Entity> gView = eManager.view<GraphicsComponent>();
  for (auto& e : gView) {
    std::shared_ptr<PositionComponent> p =
      eManager.getComponent<PositionComponent>(e);
    if (p) {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(p->model));
    } else {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(glm::identity<glm::mat4>()));
    }

    std::shared_ptr<GraphicsComponent> g =
      eManager.getComponent<GraphicsComponent>(e);

    g->m_grapObj->drawGeom(p_shaderProgram);
  }

  glCullFace(GL_BACK);
}

void
ShadowPass::setViewport(u32 w, u32 h)
{
  p_width = w;
  p_height = h;

  p_fboManager.bindFBO("depthMapFbo");
  p_textureManager.bindTexture("depthMap");
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_DEPTH_COMPONENT24,
               p_width,
               p_height,
               0,
               GL_DEPTH_COMPONENT,
               GL_UNSIGNED_INT,
               0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
