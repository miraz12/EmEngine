#ifndef GRAPHICSCOMPONENT_H_
#define GRAPHICSCOMPONENT_H_

#include "Component.hpp"
#include "Objects/GraphicsObject.hpp"

class GraphicsComponent : public Component {
public:
  GraphicsComponent(GraphicsObject &grapComp);
  GraphicsComponent() = delete;
  virtual ~GraphicsComponent();

  GraphicsObject &m_grapObj;
};
#endif // GRAPHICSCOMPONENT_H_
