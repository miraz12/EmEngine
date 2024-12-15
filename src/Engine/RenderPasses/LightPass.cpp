#include "LightPass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <ECS/Components/LightingComponent.hpp>
#include <Managers/TextureManager.hpp>

#include <ECS/ECSManager.hpp>

LightPass::LightPass()
    : RenderPass("resources/Shaders/lightVertex.glsl",
                 "resources/Shaders/pbrLightFragment.glsl") {

  p_shaderProgram.setUniformBinding("debugView");
  p_shaderProgram.setUniformBinding("gPositionAo");
  p_shaderProgram.setUniformBinding("gNormalMetal");
  p_shaderProgram.setUniformBinding("gAlbedoSpecRough");
  p_shaderProgram.setUniformBinding("gEmissive");
  p_shaderProgram.setUniformBinding("nrOfPointLights");
  p_shaderProgram.setUniformBinding("camPos");
  p_shaderProgram.setUniformBinding("directionalLight.direction");
  p_shaderProgram.setUniformBinding("directionalLight.color");
  p_shaderProgram.setUniformBinding("directionalLight.ambientIntensity");
  p_shaderProgram.setUniformBinding("lightSpaceMatrix");
  p_shaderProgram.setUniformBinding("depthMap");
  p_shaderProgram.setUniformBinding("irradianceMap");
  p_shaderProgram.setUniformBinding("prefilterMap");
  p_shaderProgram.setUniformBinding("brdfLUT");

  p_shaderProgram.use();

  glGenFramebuffers(1, &m_lightBuffer);
  glGenRenderbuffers(1, &m_rbo);
  p_fboManager.setFBO("lightFBO", m_lightBuffer);
  setViewport(p_width, p_height);

  for (u32 i = 0; i < 10; i++) {
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].position");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].color");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].diffuseIntensity");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].constant");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].linear");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].quadratic");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) +
                                      "].radius");
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
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
}

void LightPass::Execute(ECSManager &eManager) {
  p_fboManager.bindFBO("lightFBO");
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  p_shaderProgram.use();
  glUniform1i(p_shaderProgram.getUniformLocation("debugView"),
              eManager.getDebugView());

  glm::mat4 lightProjection;
  glm::mat4 lightView;
  glm::mat4 lightSpaceMatrix;
  float shadowBox = 9.0f;
  lightProjection =
      glm::ortho(-shadowBox, shadowBox, -shadowBox, shadowBox, 1.0f, 30.0f);
  glm::vec3 lightInvDir = -glm::normalize(eManager.dDir) * 20.0f;
  lightView = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  lightSpaceMatrix = lightProjection * lightView;
  glUniformMatrix4fv(p_shaderProgram.getUniformLocation("lightSpaceMatrix"), 1,
                     GL_FALSE, glm::value_ptr(lightSpaceMatrix));

  std::vector<Entity> view = eManager.view<LightingComponent>();
  i32 numPLights = 0;
  for (auto e : view) {
    std::shared_ptr<LightingComponent> g =
        eManager.getComponent<LightingComponent>(e);

    LightingComponent::TYPE t = g->getType();
    if (t == LightingComponent::TYPE::DIRECTIONAL) {
      DirectionalLight &light =
          static_cast<DirectionalLight &>(g->getBaseLight());
      glUniform3fv(
          p_shaderProgram.getUniformLocation("directionalLight.direction"), 1,
          glm::value_ptr(light.direction));

      glUniform3fv(p_shaderProgram.getUniformLocation("directionalLight.color"),
                   1, glm::value_ptr(light.color));

      glUniform1f(p_shaderProgram.getUniformLocation(
                      "directionalLight.ambientIntensity"),
                  light.ambientIntensity);

    } else if (t == LightingComponent::TYPE::POINT) {
      PointLight &light = static_cast<PointLight &>(g->getBaseLight());
      glUniform3fv(
          p_shaderProgram.getUniformLocation(
              "pointLights[" + std::to_string(numPLights) + "].position"),
          1, glm::value_ptr(light.position));

      glUniform3fv(p_shaderProgram.getUniformLocation(
                       "pointLights[" + std::to_string(numPLights) + "].color"),
                   1, glm::value_ptr(light.color));

      glUniform1f(
          p_shaderProgram.getUniformLocation(
              "pointLights[" + std::to_string(numPLights) + "].constant"),
          light.constant);

      glUniform1f(p_shaderProgram.getUniformLocation(
                      "pointLights[" + std::to_string(numPLights) + "].linear"),
                  light.linear);

      glUniform1f(
          p_shaderProgram.getUniformLocation(
              "pointLights[" + std::to_string(numPLights) + "].quadratic"),
          light.quadratic);
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
      glUniform1f(p_shaderProgram.getUniformLocation(
                      "pointLights[" + std::to_string(numPLights) + "].radius"),
                  radius);

      numPLights++;
    }
  }

  glUniform1i(p_shaderProgram.getUniformLocation("nrOfPointLights"),
              numPLights);

  auto cam = static_pointer_cast<CameraComponent>(
      ECSManager::getInstance().getCamera());
  glUniform3fv(p_shaderProgram.getUniformLocation("camPos"), 1,
               glm::value_ptr(cam->m_position));

  glBindVertexArray(quadVAO);
  for (size_t i = 0; i < p_textures.size(); i++) {
    p_textureManager.bindActivateTexture(p_textures[i], i);
  }
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void LightPass::setViewport(u32 w, u32 h) {
  p_width = w;
  p_height = h;

  p_fboManager.bindFBO("lightFBO");

  // - position color buffer
  u32 lightFrame = p_textureManager.loadTexture(
      "lightFrame", GL_RGBA16F, GL_RGBA, GL_FLOAT, p_width, p_height, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         lightFrame, 0);

  u32 attachments[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachments);
  glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, p_width,
                        p_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, m_rbo);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
