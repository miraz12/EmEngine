#ifndef BLOOMPASS_H_
#define BLOOMPASS_H_

#include "RenderPasses/RenderPass.hpp"

class BloomPass : public RenderPass {
public:
  BloomPass();
  virtual ~BloomPass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(u32 w, u32 h) override;

private:
  struct mipLevel {
    glm::vec2 size;
    glm::ivec2 intSize;
    u32 texture;
  };

  std::vector<mipLevel> m_mipChain;
  ShaderProgram m_extractBright, m_downShader, m_bloomCombine;
};

#endif // BLOOMPASS_H_
