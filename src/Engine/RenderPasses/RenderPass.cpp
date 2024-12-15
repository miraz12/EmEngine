#include "RenderPass.hpp"

RenderPass::RenderPass(std::string vs, std::string fs)
  : p_shaderProgram(vs, fs)
{
}

void
RenderPass::addTexture(std::string texName)
{
  p_shaderProgram.use();
  glUniform1i(p_shaderProgram.getUniformLocation(texName), p_textures.size());
  p_textures.push_back(texName);
};
