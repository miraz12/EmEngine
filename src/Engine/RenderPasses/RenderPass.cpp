#include "RenderPass.hpp"
#include <Graphics/GraphicsDevice.hpp>

RenderPass::RenderPass(std::string_view name,
                       std::string_view vs,
                       std::string_view fs)
  : m_shaderName(name)
{
  auto& resources = gfx::RenderResources::getInstance();
  resources.loadShaderProgram(m_shaderName, std::string(vs), std::string(fs));
}

void
RenderPass::addTexture(std::string_view texName)
{
  useShader();
  auto& device = gfx::GraphicsDevice::getInstance();
  device.setUniformInt(getUniformLocation(std::string(texName).c_str()),
                       static_cast<i32>(m_textures.size()));
  m_textures.push_back(std::string(texName));
}

void
RenderPass::useShader()
{
  gfx::RenderResources::getInstance().bindShaderProgram(m_shaderName);
}

i32
RenderPass::getUniformLocation(const char* name) const
{
  return gfx::RenderResources::getInstance().getUniformLocation(m_shaderName,
                                                                name);
}

i32
RenderPass::getAttribLocation(const char* name) const
{
  return gfx::RenderResources::getInstance().getAttribLocation(m_shaderName,
                                                               name);
}

gfx::ShaderId
RenderPass::getShaderId() const
{
  return gfx::RenderResources::getInstance().getShaderProgram(m_shaderName);
}

gfx::CommandBuffer*
RenderPass::getCommandBuffer()
{
  auto& device = gfx::GraphicsDevice::getInstance();

  // Create command buffer if needed
  if (!m_cmdBuffer.isValid()) {
    m_cmdBuffer = device.createCommandBuffer();
  }

  // Reset for new frame recording
  gfx::CommandBuffer* cmd = device.getCommandBuffer(m_cmdBuffer);
  if (cmd != nullptr) {
    cmd->reset();
  }
  return cmd;
}
