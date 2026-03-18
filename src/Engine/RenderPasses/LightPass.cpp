#include "LightPass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "LightingUtil.hpp"
#include <ECS/Components/LightingComponent.hpp>
#include <Managers/TextureManager.hpp>

#include <ECS/ECSManager.hpp>

LightPass::LightPass()
  : RenderPass("resources/Shaders/light.vert",
               "resources/Shaders/pbrLight.frag")
{

  m_shaderProgram.setUniformBinding("debugView");
  m_shaderProgram.setUniformBinding("gPositionAo");
  m_shaderProgram.setUniformBinding("gNormalMetal");
  m_shaderProgram.setUniformBinding("gAlbedoSpecRough");
  m_shaderProgram.setUniformBinding("gEmissive");
  m_shaderProgram.setUniformBinding("nrOfPointLights");
  m_shaderProgram.setUniformBinding("camPos");
  m_shaderProgram.setUniformBinding("viewMatrix");
  m_shaderProgram.setUniformBinding("directionalLight.direction");
  m_shaderProgram.setUniformBinding("directionalLight.color");
  m_shaderProgram.setUniformBinding("directionalLight.intensity");
  m_shaderProgram.setUniformBinding("depthMapArray");
  m_shaderProgram.setUniformBinding("irradianceMap");
  m_shaderProgram.setUniformBinding("prefilterMap");
  m_shaderProgram.setUniformBinding("brdfLUT");

  m_shaderProgram.use();

  // Bind UBO block for cascade data (binding point 0)
  u32 blockIndex = glGetUniformBlockIndex(m_shaderProgram.getId(), "CascadeData");
  glUniformBlockBinding(m_shaderProgram.getId(), blockIndex, 0);

  glGenFramebuffers(1, &m_lightBuffer);
  glGenRenderbuffers(1, &m_rbo);
  m_fboManager.setFBO("lightFBO", m_lightBuffer);
  setViewport(m_width, m_height);

  for (u32 i = 0; i < MAX_POINT_LIGHTS; i++) {
    m_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].position");
    m_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].color");
    m_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].diffuseIntensity");
    m_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].constant");
    m_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].linear");
    m_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].quadratic");
    m_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].radius");
  }

  // Cache uniform locations for performance
  m_debugViewLoc = m_shaderProgram.getUniformLocation("debugView");
  m_nrOfPointLightsLoc = m_shaderProgram.getUniformLocation("nrOfPointLights");
  m_camPosLoc = m_shaderProgram.getUniformLocation("camPos");
  m_viewMatrixLoc = m_shaderProgram.getUniformLocation("viewMatrix");
  m_dirLightDirectionLoc =
    m_shaderProgram.getUniformLocation("directionalLight.direction");
  m_dirLightColorLoc =
    m_shaderProgram.getUniformLocation("directionalLight.color");
  m_dirLightIntensityLoc =
    m_shaderProgram.getUniformLocation("directionalLight.intensity");

  for (u32 i = 0; i < MAX_POINT_LIGHTS; i++) {
    m_pointLightPositionLocs[i] = m_shaderProgram.getUniformLocation(
      "pointLights[" + std::to_string(i) + "].position");
    m_pointLightColorLocs[i] = m_shaderProgram.getUniformLocation(
      "pointLights[" + std::to_string(i) + "].color");
    m_pointLightConstantLocs[i] = m_shaderProgram.getUniformLocation(
      "pointLights[" + std::to_string(i) + "].constant");
    m_pointLightLinearLocs[i] = m_shaderProgram.getUniformLocation(
      "pointLights[" + std::to_string(i) + "].linear");
    m_pointLightQuadraticLocs[i] = m_shaderProgram.getUniformLocation(
      "pointLights[" + std::to_string(i) + "].quadratic");
    m_pointLightRadiusLocs[i] = m_shaderProgram.getUniformLocation(
      "pointLights[" + std::to_string(i) + "].radius");
  }

  u32 quadVBO;
  float quadVertices[] = {
    // positions        // texture Coords
    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
  };

  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);

  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void
LightPass::Execute(ECSManager& eManager)
{
#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Lightning Pass");
#endif
  m_fboManager.bindFBO("lightFBO");
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  m_shaderProgram.use();
  glUniform1i(m_debugViewLoc, eManager.getDebugView());

  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  std::vector<Entity> view = eManager.view<LightingComponent>();
  i32 numPLights = 0;
  for (auto e : view) {
    std::shared_ptr<LightingComponent> g =
      eManager.getComponent<LightingComponent>(e);

    switch (g->getType()) {
      case LightingComponent::TYPE::DIRECTIONAL: {
        auto& light = static_cast<DirectionalLight&>(g->getBaseLight());

        glUniform3fv(m_dirLightDirectionLoc, 1, glm::value_ptr(light.direction));
        glUniform3fv(m_dirLightColorLoc, 1, glm::value_ptr(light.color));
        glUniform1f(m_dirLightIntensityLoc, light.intensity);

        break;
      }
      case LightingComponent::TYPE::POINT: {

        PointLight& light = static_cast<PointLight&>(g->getBaseLight());
        glUniform3fv(m_pointLightPositionLocs[numPLights], 1,
                     glm::value_ptr(light.position));
        glUniform3fv(m_pointLightColorLocs[numPLights], 1,
                     glm::value_ptr(light.color));
        glUniform1f(m_pointLightConstantLocs[numPLights], light.constant);
        glUniform1f(m_pointLightLinearLocs[numPLights], light.linear);
        glUniform1f(m_pointLightQuadraticLocs[numPLights], light.quadratic);

        const float constant =
          1.0f; // note that we don't send this to the shader, we assume it is
                // always 1.0 (in our case)
        float maxBrightness =
          std::fmaxf(std::fmaxf(light.color.r, light.color.g), light.color.b);
        float radius =
          (-light.linear +
           std::sqrt(light.linear * light.linear -
                     4 * light.quadratic *
                       (constant - (256.0f / 5.0f) * maxBrightness))) /
          (2.0f * light.quadratic);
        glUniform1f(m_pointLightRadiusLocs[numPLights], radius);

        numPLights++;
        break;
      }
      default:
        break;
    }
  }

  glUniform1i(m_nrOfPointLightsLoc, numPLights);
  glUniform3fv(m_camPosLoc, 1, glm::value_ptr(cam->m_position));
  glUniformMatrix4fv(m_viewMatrixLoc, 1, GL_FALSE,
                     glm::value_ptr(cam->m_viewMatrix));

  glBindVertexArray(quadVAO);
  for (size_t i = 0; i < m_textures.size(); i++) {
    m_textureManager.bindActivateTexture(m_textures[i], i);
  }
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // Clean up GL state
  glBindVertexArray(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPopDebugGroup();
#endif
}

void
LightPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  m_fboManager.bindFBO("lightFBO");

  // - position color buffer
  u32 lightFrame = m_textureManager.loadTexture(
    "lightFrame", GL_RGBA16F, GL_RGBA, GL_FLOAT, m_width, m_height, 0);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightFrame, 0);

  u32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, attachments);
  glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
  glRenderbufferStorage(
    GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
