#ifndef RENDERPASS_H_
#define RENDERPASS_H_
#include "Managers/FrameBufferManager.hpp"
#include "ShaderPrograms/ShaderProgram.hpp"
#include <Managers/TextureManager.hpp>

class ECSManager;
class FrameGraph;

class RenderPass
{
public:
  RenderPass() = delete;
  RenderPass(std::string vs, std::string fs);
  virtual ~RenderPass() = default;
  virtual void Execute(ECSManager& eManager) = 0;
  virtual void setViewport(u32 /* w */, u32 /* h */) = 0;
  virtual void Init(FrameGraph& /* fGraph */) = 0;
  void addTexture(std::string texName);

protected:
  u32 p_width{ 800 };
  u32 p_height{ 800 };
  ShaderProgram p_shaderProgram;
  FrameBufferManager& p_fboManager{ FrameBufferManager::getInstance() };
  TextureManager& p_textureManager{ TextureManager::getInstance() };
  std::vector<std::string> p_textures;
};

#endif // RENDERPASS_H_
