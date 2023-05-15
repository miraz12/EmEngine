#include "Material.hpp"
#include <iostream>
#include <ostream>

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

Material::Material() : m_textureMan(TextureManager::getInstance()) {}

void Material::bind(const ShaderProgram &sPrg) {

  std::cout << "Bind textures" << std::endl;

  glUniform1i(sPrg.getUniformLocation("material"), m_material);

  glUniform3f(sPrg.getUniformLocation("baseColorFactor"), m_baseColorFactor.x, m_baseColorFactor.y,
              m_baseColorFactor.z);

  glUniform1f(sPrg.getUniformLocation("roughnessFactor"), m_roughnessFactor);
  glUniform1f(sPrg.getUniformLocation("metallicFactor"), m_metallicFactor);

  glUniform3f(sPrg.getUniformLocation("emissiveFactor"), m_emissiveFactor.x, m_emissiveFactor.y,
              m_emissiveFactor.z);

  GLint tex[5] = {0, 1, 2, 3, 4};
  glUniform1iv(sPrg.getUniformLocation("textures"), 5, tex);

  if ((m_material & (1 << 0)) > 0) {
    glActiveTexture(GL_TEXTURE0 + 0);
    std::cout << "Bount tex0" << std::endl;
    m_textureMan.bindTexture(m_baseColorTexture);
  }
  if ((m_material & (1 << 1)) > 0) {
    glActiveTexture(GL_TEXTURE0 + 1);
    std::cout << "Bount tex1" << std::endl;
    m_textureMan.bindTexture(m_metallicRoughnessTexture);
  }
  if ((m_material & (1 << 2)) > 0) {
    glActiveTexture(GL_TEXTURE0 + 2);
    std::cout << "Bount tex2" << std::endl;
    m_textureMan.bindTexture(m_emissiveTexture);
  }
  if ((m_material & (1 << 3)) > 0) {
    glActiveTexture(GL_TEXTURE0 + 3);
    std::cout << "Bount tex3" << std::endl;
    m_textureMan.bindTexture(m_occlusionTexture);
  }
  // if ((m_material & (1 << 4)) > 0) {
  //   glActiveTexture(GL_TEXTURE0 + 4);
  //   m_textureMan.bindTexture(m_normalTexture);
  // }

  if (m_doubleSided) {
    glDisable(GL_CULL_FACE);
  } else {
    glEnable(GL_CULL_FACE);
  }
  if (m_alphaMode == "BLEND") {
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
  } else {
    glDisable(GL_BLEND);
  }
}
