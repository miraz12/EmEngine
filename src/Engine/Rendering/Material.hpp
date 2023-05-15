#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <Managers/TextureManager.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Material {
public:
  Material();
  ~Material() = default;
  void bind(const ShaderProgram &sPrg);

  TextureManager &m_textureMan;
  int m_material = -1;
  glm::vec3 m_emissiveFactor = glm::vec3(1.0f);
  glm::vec3 m_baseColorFactor = glm::vec3(1.0f);
  float m_roughnessFactor = 1.0f;
  float m_metallicFactor = 1.0f;
  bool m_doubleSided;
  std::string m_alphaMode;
  std::vector<std::string> m_textures;

  std::string m_baseColorTexture;
  std::string m_metallicRoughnessTexture;
  std::string m_emissiveTexture;
  std::string m_occlusionTexture;
  std::string m_normalTexture;
};

#endif // MATERIAL_H_
