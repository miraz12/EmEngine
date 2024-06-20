#ifndef GRAPHICSCOMPONENT_H_
#define GRAPHICSCOMPONENT_H_

#include "Component.hpp"
#include "Objects/GraphicsObject.hpp"

class GraphicsComponent final : public Component {
public:
  explicit GraphicsComponent(std::shared_ptr<GraphicsObject> grapComp)
      : m_grapObj(grapComp) {}
  ~GraphicsComponent() final = default;

  GraphicsComponent() = delete;

  std::shared_ptr<GraphicsObject> m_grapObj;

  enum class TYPE { POINT, LINE, QUAD, CUBE, MESH, HEIGHTMAP } type;
};
#endif // GRAPHICSCOMPONENT_H_
