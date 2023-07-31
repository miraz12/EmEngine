#ifndef DEBUGCOMPONENT_H_
#define DEBUGCOMPONENT_H_

#include "Component.hpp"
#include "Objects/GraphicsObject.hpp"

class DebugComponent : public Component {
public:
  DebugComponent(GraphicsObject *grapComp) : m_grapObj(grapComp){};
  DebugComponent() = delete;
  ~DebugComponent() = default;

  std::unique_ptr<GraphicsObject> m_grapObj;
};

#endif // DEBUGCOMPONENT_H_
