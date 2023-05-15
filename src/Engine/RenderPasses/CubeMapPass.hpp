// #ifndef CUBEMAPPASS_H_
// #define CUBEMAPPASS_H_

// #include "Managers/FrameBufferManager.hpp"
// #include "RenderPasses/RenderPass.hpp"
// #include <Camera.hpp>
// #include <Managers/TextureManager.hpp>



// class CubeMapPass : public RenderPass {
// public:
//   CubeMapPass();
//   virtual ~CubeMapPass(){};
//   void Execute(ECSManager &eManager) override;
//   void setViewport(unsigned int w, unsigned int h) override;

// private:
//   void renderCube();
//   unsigned int m_captureFBO, m_captureRBO, m_cubeVAO, m_cubeVBO, m_envCubemap;
//   ShaderProgram m_equirectangularToCubemapShader, m_irradianceShader;
// };

// #endif // CUBEMAPPASS_H_
