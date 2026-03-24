#include "Material.hpp"

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/RenderResources.hpp>
#include <Graphics/UBOStructs.hpp>

void
Material::bind(gfx::ShaderId /* shader */)
{
  auto& resources = gfx::RenderResources::getInstance();

  // Populate MaterialUBO
  auto& matUBO = resources.getMaterialUBO();
  matUBO.baseColorFactor = glm::vec4(m_baseColorFactor, 1.0f);
  matUBO.emissiveFactor = glm::vec4(m_emissiveFactor, 0.0f);
  matUBO.pbrFactors =
    glm::vec4(m_roughnessFactor, m_metallicFactor, m_alphaCutoff, 0.0f);

  i32 alphaModeInt = (m_alphaMode == "BLEND")  ? 0
                     : (m_alphaMode == "MASK") ? 1
                                               : 2;
  matUBO.materialConfig = glm::ivec4(m_material, alphaModeInt, 0, 0);

  resources.flushMaterialUBO();

  // Texture binding (already abstracted)
  resources.bindTexture(0, m_baseColorTexture);
  resources.bindTexture(1, m_metallicRoughnessTexture);
  resources.bindTexture(2, m_emissiveTexture);
  resources.bindTexture(3, m_occlusionTexture);
  resources.bindTexture(4, m_normalTexture);

  // Render state (already abstracted)
  if (m_doubleSided) {
    resources.setCullMode(gfx::CullMode::None);
  } else {
    resources.setCullMode(gfx::CullMode::Back);
  }

  if (m_alphaMode == "BLEND") {
    resources.setBlendEnabled(true);
    resources.setBlendFunc(gfx::BlendFactor::SrcAlpha,
                           gfx::BlendFactor::OneMinusSrcAlpha);
  } else if (m_alphaMode == "OPAQUE") {
    resources.setBlendEnabled(false);
    resources.setColorMask(true, true, true, true);
    resources.setBlendColor(1.0f, 1.0f, 1.0f, 1.0f);
    resources.setBlendFunc(gfx::BlendFactor::ConstantAlpha,
                           gfx::BlendFactor::OneMinusConstantAlpha);
  }
  // MASK mode: no blend state changes needed (alpha test in shader)
}

void
Material::recordBind(gfx::CommandBuffer& cmd, gfx::SamplerId sampler)
{
  // Build MaterialUBO data locally and record it in the command stream
  // This ensures each material's data is stored inline and flushed at execution
  // time
  gfx::MaterialUBO matUBO{};
  matUBO.baseColorFactor = glm::vec4(m_baseColorFactor, 1.0f);
  matUBO.emissiveFactor = glm::vec4(m_emissiveFactor, 0.0f);
  matUBO.pbrFactors =
    glm::vec4(m_roughnessFactor, m_metallicFactor, m_alphaCutoff, 0.0f);

  i32 alphaModeInt = (m_alphaMode == "BLEND")  ? 0
                     : (m_alphaMode == "MASK") ? 1
                                               : 2;
  matUBO.materialConfig = glm::ivec4(m_material, alphaModeInt, 0, 0);

  // Record UBO update command (stores data in command stream for deferred
  // execution)
  cmd.updateMaterialUBO(matUBO);

  // Record texture bindings by name
  cmd.bindTextureByName(0, m_baseColorTexture, sampler);
  cmd.bindTextureByName(1, m_metallicRoughnessTexture, sampler);
  cmd.bindTextureByName(2, m_emissiveTexture, sampler);
  cmd.bindTextureByName(3, m_occlusionTexture, sampler);
  cmd.bindTextureByName(4, m_normalTexture, sampler);

  // Record render state
  if (m_doubleSided) {
    cmd.setCullMode(gfx::CullMode::None);
  } else {
    cmd.setCullMode(gfx::CullMode::Back);
  }

  if (m_alphaMode == "BLEND") {
    cmd.setBlendEnabled(true);
    cmd.setBlendFunc(gfx::BlendFactor::SrcAlpha,
                     gfx::BlendFactor::OneMinusSrcAlpha,
                     gfx::BlendFactor::SrcAlpha,
                     gfx::BlendFactor::OneMinusSrcAlpha);
  } else if (m_alphaMode == "OPAQUE") {
    cmd.setBlendEnabled(false);
    cmd.setBlendFunc(gfx::BlendFactor::ConstantAlpha,
                     gfx::BlendFactor::OneMinusConstantAlpha,
                     gfx::BlendFactor::ConstantAlpha,
                     gfx::BlendFactor::OneMinusConstantAlpha);
  }
  // MASK mode: no blend state changes needed (alpha test in shader)
}
