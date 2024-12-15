#ifndef CUBEMAPPASS_H_
#define CUBEMAPPASS_H_

#include "RenderPasses/RenderPass.hpp"
#include <Managers/TextureManager.hpp>

class CubeMapPass final : public RenderPass
{
public:
  CubeMapPass();
  ~CubeMapPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& fGraph) override;

private:
  void generateCubeMap();
  void generateIrradianceMap();
  void generatePrefilterMap();
  void generateBRDF();

  u32 m_cubeBuffer;
  u32 m_rbo;
  u32 m_captureFBO{ 0 };
  u32 m_captureRBO{ 0 };
  u32 m_envCubemap{ 0 };
  ShaderProgram m_equirectangularToCubemapShader;
  ShaderProgram m_irradianceShader;
  ShaderProgram m_prefilterShader;
  ShaderProgram m_brdfShader;

  glm::mat4 m_captureProjection{
    glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f)
  };
  glm::mat4 m_captureViews[6] = { glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                              glm::vec3(1.0f, 0.0f, 0.0f),
                                              glm::vec3(0.0f, -1.0f, 0.0f)),
                                  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                              glm::vec3(-1.0f, 0.0f, 0.0f),
                                              glm::vec3(0.0f, -1.0f, 0.0f)),
                                  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                              glm::vec3(0.0f, 1.0f, 0.0f),
                                              glm::vec3(0.0f, 0.0f, 1.0f)),
                                  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                              glm::vec3(0.0f, -1.0f, 0.0f),
                                              glm::vec3(0.0f, 0.0f, -1.0f)),
                                  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                              glm::vec3(0.0f, 0.0f, 1.0f),
                                              glm::vec3(0.0f, -1.0f, 0.0f)),
                                  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                              glm::vec3(0.0f, 0.0f, -1.0f),
                                              glm::vec3(0.0f, -1.0f, 0.0f)) };
};

#endif // CUBEMAPPASS_H_
