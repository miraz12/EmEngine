#ifndef BLOOMPASS_H_
#define BLOOMPASS_H_

#include "RenderPasses/RenderPass.hpp"
#include <glm/glm.hpp>

class BloomPass : public RenderPass {
public:
  BloomPass();
  virtual ~BloomPass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(uint32_t w, uint32_t h) override;

private:
  void renderQuad();

  struct mipLevel {
    glm::vec2 size;
    glm::ivec2 intSize;
    unsigned int texture;
  };

  std::vector<mipLevel> m_mipChain;
  ShaderProgram m_downShader;
  uint32_t m_quadVAO{0};
  uint32_t m_quadVBO{0};
};

#endif // BLOOMPASS_H_
