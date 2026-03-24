#ifndef CUBEMAPPASS_H_
#define CUBEMAPPASS_H_

#include "RenderPasses/RenderPass.hpp"

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

  // Shader names (loaded via RenderResources)
  std::string m_equirectToCubeName{ "EquirectToCube" };
  std::string m_irradianceName{ "Irradiance" };
  std::string m_prefilterName{ "Prefilter" };
  std::string m_brdfName{ "BRDF" };

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

  // Pipeline for CommandBuffer background cube rendering
  gfx::PipelineId m_backgroundPipeline;
};

#endif // CUBEMAPPASS_H_
