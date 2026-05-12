#ifndef GRAPHICSCOMPONENT_H_
#define GRAPHICSCOMPONENT_H_

#include "Objects/GraphicsObject.hpp"

struct GraphicsComponent
{
  explicit GraphicsComponent(std::shared_ptr<GraphicsObject> grapComp)
    : m_grapObj(std::move(grapComp))
  {
  }
  ~GraphicsComponent() = default;

  GraphicsComponent() = delete;

  std::shared_ptr<GraphicsObject> m_grapObj;

  enum class TYPE
  {
    POINT,
    LINE,
    QUAD,
    CUBE,
    MESH,
    HEIGHTMAP
  } type;
};
#endif // GRAPHICSCOMPONENT_H_
