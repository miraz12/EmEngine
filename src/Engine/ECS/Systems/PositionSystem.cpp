#include "PositionSystem.hpp"

void
PositionSystem::update(float /* dt */)
{
  // Model matrix computation moved to render passes (GeometryPass, ShadowPass)
  // to keep PositionComponent small and cache-friendly.
}
