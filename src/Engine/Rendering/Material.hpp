#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <Graphics/Handle.hpp>

namespace gfx {
class CommandBuffer;
}

class Material
{
public:
  Material() = default;
  ~Material() = default;
  void bind(gfx::ShaderId shader);
  /// Record material binding commands (textures, UBO, render state) into
  /// CommandBuffer
  void recordBind(gfx::CommandBuffer& cmd, gfx::SamplerId sampler);
  i32 m_material{ 0 };
  glm::vec3 m_emissiveFactor = glm::vec3(0.0f);
  glm::vec3 m_baseColorFactor = glm::vec3(1.0f);
  float m_roughnessFactor = 1.0f;
  float m_metallicFactor = 1.0f;
  bool m_doubleSided{ false };
  float m_alphaCutoff{ 0.5 };
  std::string m_alphaMode{ "OPAQUE" };
  std::vector<std::string> m_textures;

  std::string m_baseColorTexture{ "black_default" };
  std::string m_metallicRoughnessTexture{ "black_default" };
  std::string m_emissiveTexture{ "black_default" };
  std::string m_occlusionTexture{ "black_default" };
  std::string m_normalTexture{ "black_default" };
};

#endif // MATERIAL_H_
