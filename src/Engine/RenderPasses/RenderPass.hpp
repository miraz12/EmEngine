#ifndef RENDERPASS_H_
#define RENDERPASS_H_
#include "Managers/FrameBufferManager.hpp"
#include "ShaderPrograms/ShaderProgram.hpp"
#include <Managers/TextureManager.hpp>

class ECSManager;

class RenderPass {
public:
  RenderPass() = delete;
  RenderPass(std::string vs, std::string fs);
  virtual ~RenderPass() = default;
  virtual void Execute(ECSManager &eManager) = 0;
  virtual void setViewport(unsigned int w, unsigned int h) = 0;

protected:
  unsigned int p_width{800}, p_height{800};
  ShaderProgram p_shaderProgram;
  FrameBufferManager &p_fboManager;
  TextureManager &p_textureManager;
};

#endif // RENDERPASS_H_
