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

  m_shaderProgram.use();
  m_shaderProgram.setUniformBinding("modelMatrix");
  m_shaderProgram.setUniformBinding("lightSpaceMatrix");
  m_shaderProgram.setUniformBinding("meshMatrix");
  m_shaderProgram.setUniformBinding("is_skinned");
  m_shaderProgram.setUniformBinding("jointMats");

  m_shaderProgram.setAttribBinding("POSITION");
  m_shaderProgram.setAttribBinding("JOINTS_0");
  m_shaderProgram.setAttribBinding("WEIGHTS_0");

  u32 jointMats;
  glGenTextures(1, &jointMats);
  m_textureManager.setTexture("jointMats", jointMats);

  m_textureManager.bindTexture("jointMats");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // Create UBO for cascade data
  glGenBuffers(1, &m_cascadeUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, m_cascadeUBO);
  // Allocate space for cascade UBO (4 mat4 + 1 vec4 + 1 ivec4 = 288 bytes)
  glBufferData(GL_UNIFORM_BUFFER, 288, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_cascadeUBO);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  u32 depthMapFbo;
  glGenFramebuffers(1, &depthMapFbo);
  m_fboManager.setFBO("depthMapFbo", depthMapFbo);

  u32 depthMapArray;
  glGenTextures(1, &depthMapArray);
  m_textureManager.setTexture("depthMapArray", depthMapArray, GL_TEXTURE_2D_ARRAY);

  m_fboManager.bindFBO("depthMapFbo");
  glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapArray);
  glTexImage3D(GL_TEXTURE_2D_ARRAY,
               0,
               GL_DEPTH_COMPONENT24,
               SHADOW_MAP_SIZE,
               SHADOW_MAP_SIZE,
               NUM_CASCADES,
               0,
               GL_DEPTH_COMPONENT,
               GL_UNSIGNED_INT,
               nullptr);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(
    GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

  // Attach first cascade layer for initial setup (will be rebound per cascade)
  glFramebufferTextureLayer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapArray, 0, 0);
  GLuint buffer = GL_NONE; // Emscripten strangeness
  glDrawBuffers(1, &buffer);
  glReadBuffer(GL_NONE);

  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    printf("FB error, status: 0x%x\n", Status);
  }
}

void
ShadowPass::Init(FrameGraph& fGraph)
{
  fGraph.getPass(PassId::kLight)->addTexture("depthMapArray");
}

void
ShadowPass::Execute(ECSManager& eManager)
{
#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Shadow Pass CSM");
#endif

  // Get camera for cascade calculation
  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  // Get directional light
  glm::vec3 lightDirection(0.0f, -1.0f, 0.0f);
  std::vector<Entity> lightView = eManager.view<LightingComponent>();
  for (auto lightEntity : lightView) {
    std::shared_ptr<LightingComponent> lightComp =
      eManager.getComponent<LightingComponent>(lightEntity);

    if (lightComp->getType() == LightingComponent::TYPE::DIRECTIONAL) {
      auto& light = static_cast<DirectionalLight&>(lightComp->getBaseLight());
      lightDirection = light.direction;
      break;
    }
  }

  // Calculate cascade splits and matrices
  m_cascadeConfig.calculateSplitDistances(cam->m_near, cam->m_far, 0.5f);
  m_cascadeConfig.calculateCascadeMatrices(
    lightDirection, cam->m_position, cam->m_viewMatrix, cam->m_ProjectionMatrix);

  // Update UBO with cascade data
  struct CascadeUBO
  {
    glm::mat4 lightSpaceMatrices[NUM_CASCADES];
    glm::vec4 cascadeSplits;
    glm::ivec4 config;
  } uboData;

  std::copy(std::begin(m_cascadeConfig.lightSpaceMatrices),
            std::end(m_cascadeConfig.lightSpaceMatrices),
            std::begin(uboData.lightSpaceMatrices));

  uboData.cascadeSplits = glm::vec4(m_cascadeConfig.cascadeSplits[0],
                                      m_cascadeConfig.cascadeSplits[1],
                                      m_cascadeConfig.cascadeSplits[2],
                                      m_cascadeConfig.cascadeSplits[3]);
  uboData.config = glm::ivec4(NUM_CASCADES, SHADOW_MAP_SIZE, 0, 0);

  glBindBuffer(GL_UNIFORM_BUFFER, m_cascadeUBO);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CascadeUBO), &uboData);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // Setup render state
  m_fboManager.bindFBO("depthMapFbo");
  glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDepthMask(GL_TRUE);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  m_shaderProgram.use();

  u32 depthMapArray = m_textureManager.bindTexture("depthMapArray");

  // Render each cascade
  for (u32 cascade = 0; cascade < NUM_CASCADES; ++cascade) {
    // Attach this cascade's layer to the framebuffer
    glFramebufferTextureLayer(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapArray, 0, cascade);

    glClearDepthf(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set light space matrix for this cascade
    glUniformMatrix4fv(
      m_shaderProgram.getUniformLocation("lightSpaceMatrix"),
      1,
      GL_FALSE,
      glm::value_ptr(m_cascadeConfig.lightSpaceMatrices[cascade]));

    // Render all shadow-casting geometry
    std::vector<Entity> gView = eManager.view<GraphicsComponent>();
    for (auto& entity : gView) {
      std::shared_ptr<PositionComponent> posComp =
        eManager.getComponent<PositionComponent>(entity);

      if (posComp) {
        glUniformMatrix4fv(m_shaderProgram.getUniformLocation("modelMatrix"),
                           1,
                           GL_FALSE,
                           glm::value_ptr(posComp->model));
      } else {
        glUniformMatrix4fv(m_shaderProgram.getUniformLocation("modelMatrix"),
                           1,
                           GL_FALSE,
                           glm::value_ptr(glm::identity<glm::mat4>()));
      }

      std::shared_ptr<GraphicsComponent> grapComp =
        eManager.getComponent<GraphicsComponent>(entity);

      grapComp->m_grapObj->drawGeom(m_shaderProgram);
    }
  }

  // Clean up GL state
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPopDebugGroup();
#endif
}

void
ShadowPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  // Note: Shadow map resolution is fixed at SHADOW_MAP_SIZE, not viewport size
  m_fboManager.bindFBO("depthMapFbo");
  u32 depthMapArray = m_textureManager.bindTexture("depthMapArray");
  glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapArray);
  glTexImage3D(GL_TEXTURE_2D_ARRAY,
               0,
               GL_DEPTH_COMPONENT24,
               SHADOW_MAP_SIZE,
               SHADOW_MAP_SIZE,
               NUM_CASCADES,
               0,
               GL_DEPTH_COMPONENT,
               GL_UNSIGNED_INT,
               nullptr);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
