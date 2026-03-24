#ifndef RENDERUTIL_H_
#define RENDERUTIL_H_

#include <Graphics/RenderResources.hpp>

namespace Util {

/// Render a fullscreen quad using shared geometry from RenderResources.
/// Caller is responsible for binding shader and setting uniforms.
inline void
renderQuad()
{
  gfx::RenderResources::getInstance().renderQuad();
}

/// Render a unit cube using shared geometry from RenderResources.
/// Caller is responsible for binding shader and setting uniforms.
inline void
renderCube()
{
  gfx::RenderResources::getInstance().renderCube();
}

} // namespace Util

#endif // RENDERUTIL_H_
