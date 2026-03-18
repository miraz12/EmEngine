#include "RenderPass.hpp"

RenderPass::RenderPass(std::string_view vs, std::string_view fs)
  : m_shaderProgram(std::string(vs), std::string(fs))
{
}

void
RenderPass::addTexture(std::string_view texName)
{
  m_shaderProgram.use();
  glUniform1i(m_shaderProgram.getUniformLocation(std::string(texName)), static_cast<GLint>(m_textures.size()));
  m_textures.push_back(std::string(texName));
}
