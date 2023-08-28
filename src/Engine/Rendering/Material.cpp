#include "Material.hpp"
#include <iostream>
#include <ostream>

Material::Material() : m_textureMan(TextureManager::getInstance()) {}

void Material::bind(const ShaderProgram &sPrg) {

  glUniform1i(sPrg.getUniformLocation("material"), m_material);

  glUniform3f(sPrg.getUniformLocation("baseColorFactor"), m_baseColorFactor.x,
              m_baseColorFactor.y, m_baseColorFactor.z);

  glUniform1f(sPrg.getUniformLocation("roughnessFactor"), m_roughnessFactor);
  glUniform1f(sPrg.getUniformLocation("metallicFactor"), m_metallicFactor);

  glUniform3f(sPrg.getUniformLocation("emissiveFactor"), m_emissiveFactor.x,
              m_emissiveFactor.y, m_emissiveFactor.z);

  GLint tex[5] = {0, 1, 2, 3, 4};
  glUniform1iv(sPrg.getUniformLocation("textures"), 5, tex);

  if ((m_material & (1 << 0)) > 0) {
    glActiveTexture(GL_TEXTURE0 + 0);
    m_textureMan.bindTexture(m_baseColorTexture);
  }
  if ((m_material & (1 << 1)) > 0) {
    glActiveTexture(GL_TEXTURE0 + 1);
    m_textureMan.bindTexture(m_metallicRoughnessTexture);
  }
  if ((m_material & (1 << 2)) > 0) {
    glActiveTexture(GL_TEXTURE0 + 2);
    m_textureMan.bindTexture(m_emissiveTexture);
  }
  if ((m_material & (1 << 3)) > 0) {
    glActiveTexture(GL_TEXTURE0 + 3);
    m_textureMan.bindTexture(m_occlusionTexture);
  }
  if ((m_material & (1 << 4)) > 0) {
    glActiveTexture(GL_TEXTURE0 + 4);
    m_textureMan.bindTexture(m_normalTexture);
  }

  if (m_doubleSided) {
    glDisable(GL_CULL_FACE);
  } else {
    glEnable(GL_CULL_FACE);
  }

  if (m_alphaMode == "BLEND") {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUniform1i(sPrg.getUniformLocation("alphaMode"), 0);

  } else if (m_alphaMode == "MASK") {
    glUniform1i(sPrg.getUniformLocation("alphaMode"), 1);
    glUniform1f(sPrg.getUniformLocation("alphaCutoff"), m_alphaCutoff);
    // glEnable(GL_ALPHA_TEST);
    // glAlphaFunc(GL_GREATER, m_alphaCutoff);
    // glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  } else if (m_alphaMode == "OPAQUE") {
    glDisable(GL_BLEND);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    glUniform1i(sPrg.getUniformLocation("alphaMode"), 2);
  }
}
