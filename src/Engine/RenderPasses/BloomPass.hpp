#ifndef BLOOMPASS_H_
#define BLOOMPASS_H_

#include "RenderPasses/RenderPass.hpp"

class BloomPass final : public RenderPass
{
public:
  BloomPass();
  ~BloomPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& /* fGraph */) override {};

private:
  struct mipLevel
  {
    glm::vec2 size;
    glm::ivec2 intSize;
    u32 texture;
  };

  std::vector<mipLevel> m_mipChain;
  ShaderProgram m_extractBright;
  ShaderProgram m_downShader;
  ShaderProgram m_bloomCombine;

  // Cached uniform locations for performance
  GLint m_upFilterRadiusLoc{ -1 };
  GLint m_downSrcResolutionLoc{ -1 };
  GLint m_downMipLevelLoc{ -1 };
  GLint m_combineExposureLoc{ -1 };
  GLint m_combineSceneLoc{ -1 };
  GLint m_combineBloomBlurLoc{ -1 };
};

#endif // BLOOMPASS_H_
