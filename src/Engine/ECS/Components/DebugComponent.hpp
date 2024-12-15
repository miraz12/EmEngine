#ifndef DEBUGCOMPONENT_H_
#define DEBUGCOMPONENT_H_

#include "Component.hpp"
#include <Objects/GraphicsObject.hpp>

class DebugComponent final : public Component
{
public:
  explicit DebugComponent(std::unique_ptr<GraphicsObject> grapComp)
    : m_grapObj(std::move(grapComp)) {};
  DebugComponent() = delete;
  ~DebugComponent() final = default;

  std::unique_ptr<GraphicsObject> m_grapObj;
};

#endif // DEBUGCOMPONENT_H_
