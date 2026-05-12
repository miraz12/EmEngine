#ifndef DEBUGCOMPONENT_H_
#define DEBUGCOMPONENT_H_

#include <Objects/GraphicsObject.hpp>

struct DebugComponent
{
  explicit DebugComponent(std::unique_ptr<GraphicsObject> grapComp)
    : m_grapObj(std::move(grapComp)) {};
  DebugComponent() = delete;
  ~DebugComponent() = default;

  std::unique_ptr<GraphicsObject> m_grapObj;
};

#endif // DEBUGCOMPONENT_H_
