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
  void setViewport(unsigned int w, unsigned int h) override;

private:
  void renderCube();
  unsigned int m_captureFBO{0}, m_captureRBO{0}, m_cubeVAO{0}, m_cubeVBO{0}, m_envCubemap{0};
  ShaderProgram m_equirectangularToCubemapShader;
};

#endif // CUBEMAPPASS_H_
