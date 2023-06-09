#ifndef CUBEMAPPASS_H_
#define CUBEMAPPASS_H_

#include "Managers/FrameBufferManager.hpp"
#include "RenderPasses/RenderPass.hpp"
#include <Camera.hpp>
#include <Managers/TextureManager.hpp>

class CubeMapPass : public RenderPass {
public:
  CubeMapPass();
  virtual ~CubeMapPass(){};
  void Execute(ECSManager &eManager) override;
  void setViewport(uint32_t w, uint32_t h) override;
  void Init(FrameGraph &fGraph) override;

private:
  void generateCubeMap();
  void generateIrradianceMap();
  void generatePrefilterMap();
  void generateBRDF();
  void renderCube();
  void renderQuad();

  uint32_t m_captureFBO{0}, m_captureRBO{0}, m_cubeVAO{0}, m_cubeVBO{0}, m_quadVAO{0}, m_quadVBO{0},
      m_envCubemap{0};
  ShaderProgram m_equirectangularToCubemapShader, m_irradianceShader, m_prefilterShader,
      m_brdfShader;

  glm::mat4 m_captureProjection{glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f)};
  glm::mat4 m_captureViews[6] = {
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, -1.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f))};
};

#endif // CUBEMAPPASS_H_
