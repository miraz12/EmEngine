#ifndef GRAPHICSCOMPONENT_H_
#define GRAPHICSCOMPONENT_H_

#include "Component.hpp"
#include "Objects/GraphicsObject.hpp"

class GraphicsComponent : public Component {
public:
  GraphicsComponent() = default;
  virtual ~GraphicsComponent();

  GraphicsObject *grapObj;

};
#endif // GRAPHICSCOMPONENT_H_
