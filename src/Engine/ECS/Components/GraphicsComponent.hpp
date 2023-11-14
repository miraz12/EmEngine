#ifndef GRAPHICSCOMPONENT_H_
#define GRAPHICSCOMPONENT_H_

#include "Component.hpp"
#include "Objects/GraphicsObject.hpp"

class GraphicsComponent : public Component {
public:
  GraphicsComponent(GraphicsObject &grapComp) : m_grapObj(grapComp) {}
  virtual ~GraphicsComponent() { delete &m_grapObj; }

  GraphicsComponent() = delete;

  GraphicsObject &m_grapObj;

  enum TYPE { POINT, LINE, QUAD, CUBE, MESH, HEIGHTMAP } type;
};
#endif // GRAPHICSCOMPONENT_H_
