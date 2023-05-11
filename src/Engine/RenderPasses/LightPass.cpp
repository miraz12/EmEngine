#include "LightPass.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <Managers/TextureManager.hpp>

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#include <ECS/ECSManager.hpp>

LightPass::LightPass()
    : RenderPass("resources/Shaders/lightVertex.glsl", "resources/Shaders/pbrLightFragment.glsl") {

  p_shaderProgram.setUniformBinding("gPositionAo");
  p_shaderProgram.setUniformBinding("gNormalMetal");
  p_shaderProgram.setUniformBinding("gAlbedoSpecRough");
  p_shaderProgram.setUniformBinding("gEmissive");
  p_shaderProgram.setUniformBinding("depthMap");
  p_shaderProgram.setUniformBinding("nrOfPointLights");
  p_shaderProgram.setUniformBinding("camPos");
  p_shaderProgram.setUniformBinding("directionalLight.direction");
  p_shaderProgram.setUniformBinding("directionalLight.color");
  p_shaderProgram.setUniformBinding("directionalLight.ambientIntensity");
  p_shaderProgram.setUniformBinding("lightSpaceMatrix");

  p_shaderProgram.use();

  setViewport(p_width, p_height);

  glUniform1i(p_shaderProgram.getUniformLocation("gPositionAo"), 0);
  glUniform1i(p_shaderProgram.getUniformLocation("gNormalMetal"), 1);
  glUniform1i(p_shaderProgram.getUniformLocation("gAlbedoSpecRough"), 2);
  glUniform1i(p_shaderProgram.getUniformLocation("gEmissive"), 3);
  glUniform1i(p_shaderProgram.getUniformLocation("depthMap"), 4);

  for (unsigned int i = 0; i < 10; i++) {
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) + "].position");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) + "].color");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) + "].diffuseIntensity");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) + "].constant");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) + "].linear");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) + "].quadratic");
    p_shaderProgram.setUniformBinding("pointLights[" + std::to_string(i) + "].radius");
  }

  unsigned int quadVBO;
  float quadVertices[] = {
      // positions        // texture Coords
      -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
      1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
  };

  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);

  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
}

void LightPass::Execute(ECSManager &eManager) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  p_shaderProgram.use();

  glUniform1i(p_shaderProgram.getUniformLocation("gPositionAo"), 0);
  glUniform1i(p_shaderProgram.getUniformLocation("gNormalMetal"), 1);
  glUniform1i(p_shaderProgram.getUniformLocation("gAlbedoSpecRough"), 2);
  glUniform1i(p_shaderProgram.getUniformLocation("gEmissive"), 3);
  glUniform1i(p_shaderProgram.getUniformLocation("depthMap"), 4);

  glm::mat4 lightProjection, lightView;
  glm::mat4 lightSpaceMatrix;
  float shadowBox = 9.0f;
  lightProjection = glm::ortho(-shadowBox, shadowBox, -shadowBox, shadowBox,
                               1.0f, 30.0f);
  glm::vec3 lightInvDir = -glm::normalize(eManager.dDir) * 20.0f;
  lightView = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  lightSpaceMatrix = lightProjection * lightView;
  glUniformMatrix4fv(p_shaderProgram.getUniformLocation("lightSpaceMatrix"), 1, GL_FALSE,
                     glm::value_ptr(lightSpaceMatrix));

  std::vector<Entity> view = eManager.view<LightingComponent>();
  int numPLights = 0;
  for (auto e : view) {
    LightingComponent *g =
        static_cast<LightingComponent *>(eManager.getComponent<LightingComponent>(e));

    LightingComponent::TYPE t = g->getType();
    if (t == LightingComponent::DIRECTIONAL) {
      DirectionalLight *light = static_cast<DirectionalLight *>(g->getBaseLight());
      glUniform3fv(p_shaderProgram.getUniformLocation("directionalLight.direction"), 1,
                   glm::value_ptr(light->direction));

      glUniform3fv(p_shaderProgram.getUniformLocation("directionalLight.color"), 1,
                   glm::value_ptr(light->color));

      glUniform1f(p_shaderProgram.getUniformLocation("directionalLight.ambientIntensity"),
                  light->ambientIntensity);

    } else if (t == LightingComponent::POINT) {
      PointLight *light = static_cast<PointLight *>(g->getBaseLight());
      glUniform3fv(p_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                      "].position"),
                   1, glm::value_ptr(light->position));

      glUniform3fv(p_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                      "].color"),
                   1, glm::value_ptr(light->color));

      glUniform1f(p_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                     "].constant"),
                  light->constant);

      glUniform1f(p_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                     "].linear"),
                  light->linear);

      glUniform1f(p_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                     "].quadratic"),
                  light->quadratic);
      const float constant = 1.0f; // note that we don't send this to the shader, we assume it is
                                   // always 1.0 (in our case)
      float maxBrightness = std::fmaxf(std::fmaxf(light->color.r, light->color.g), light->color.b);
      float radius =
          (-light->linear +
           std::sqrt(light->linear * light->linear -
                     4 * light->quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) /
          (2.0f * light->quadratic);
      glUniform1f(p_shaderProgram.getUniformLocation("pointLights[" + std::to_string(numPLights) +
                                                     "].radius"),
                  radius);

      numPLights++;
    }
  }

  glUniform1i(p_shaderProgram.getUniformLocation("nrOfPointLights"), numPLights);

  glUniform3fv(p_shaderProgram.getUniformLocation("camPos"), 1,
               glm::value_ptr(eManager.getCamera().getPosition()));

  // Render to quad
  glBindVertexArray(quadVAO);
  glActiveTexture(GL_TEXTURE0);
  p_textureManager.bindTexture("gPosition");
  glActiveTexture(GL_TEXTURE1);
  p_textureManager.bindTexture("gNormal");
  glActiveTexture(GL_TEXTURE2);
  p_textureManager.bindTexture("gAlbedo");
  glActiveTexture(GL_TEXTURE3);
  p_textureManager.bindTexture("gEmissive");
  glActiveTexture(GL_TEXTURE4);
  p_textureManager.bindTexture("depthMap");

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void LightPass::setViewport(unsigned int w, unsigned int h) {
  p_width = w;
  p_height = h;
}
