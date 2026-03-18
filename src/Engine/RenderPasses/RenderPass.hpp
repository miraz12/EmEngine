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
  RenderPass(std::string_view vs, std::string_view fs);
  virtual ~RenderPass() = default;
  virtual void Execute(ECSManager& eManager) = 0;
  virtual void setViewport(u32 /* w */, u32 /* h */) = 0;
  virtual void Init(FrameGraph& /* fGraph */) = 0;
  void addTexture(std::string_view texName);

protected:
  u32 m_width{ 800 };
  u32 m_height{ 800 };
  ShaderProgram m_shaderProgram;
  FrameBufferManager& m_fboManager{ FrameBufferManager::getInstance() };
  TextureManager& m_textureManager{ TextureManager::getInstance() };
  std::vector<std::string> m_textures;
};

#endif // RENDERPASS_H_
