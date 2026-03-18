#ifndef LIGHTPASS_H_
#define LIGHTPASS_H_
#include "RenderPasses/RenderPass.hpp"
#include <Managers/TextureManager.hpp>

class LightPass final : public RenderPass
{
public:
  LightPass();
  ~LightPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& /* fGraph */) override {};

private:
  static constexpr u32 MAX_POINT_LIGHTS = 10;

  u32 quadVAO;
  u32 m_lightBuffer;
  u32 m_rbo;

  // Cached uniform locations for performance
  GLint m_debugViewLoc{ -1 };
  GLint m_nrOfPointLightsLoc{ -1 };
  GLint m_camPosLoc{ -1 };
  GLint m_viewMatrixLoc{ -1 };

  // Directional light uniforms
  GLint m_dirLightDirectionLoc{ -1 };
  GLint m_dirLightColorLoc{ -1 };
  GLint m_dirLightIntensityLoc{ -1 };

  // Point light array uniforms (indexed by light number)
  std::array<GLint, MAX_POINT_LIGHTS> m_pointLightPositionLocs;
  std::array<GLint, MAX_POINT_LIGHTS> m_pointLightColorLocs;
  std::array<GLint, MAX_POINT_LIGHTS> m_pointLightConstantLocs;
  std::array<GLint, MAX_POINT_LIGHTS> m_pointLightLinearLocs;
  std::array<GLint, MAX_POINT_LIGHTS> m_pointLightQuadraticLocs;
  std::array<GLint, MAX_POINT_LIGHTS> m_pointLightRadiusLocs;
};

#endif // LIGHTPASS_H_
