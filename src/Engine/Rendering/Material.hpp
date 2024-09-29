#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <Managers/TextureManager.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>

class Material {
public:
  Material();
  ~Material() = default;
  void bind(const ShaderProgram &sPrg);

  TextureManager &m_textureMan;
  i32 m_material{0};
  glm::vec3 m_emissiveFactor = glm::vec3(0.0f);
  glm::vec3 m_baseColorFactor = glm::vec3(1.0f);
  float m_roughnessFactor = 1.0f;
  float m_metallicFactor = 1.0f;
  bool m_doubleSided{false};
  float m_alphaCutoff{0.5};
  std::string m_alphaMode{"OPAQUE"};
  std::vector<std::string> m_textures;

  std::string m_baseColorTexture{"black_default"};
  std::string m_metallicRoughnessTexture{"black_default"};
  std::string m_emissiveTexture{"black_default"};
  std::string m_occlusionTexture{"black_default"};
  std::string m_normalTexture{};
};

#endif // MATERIAL_H_
