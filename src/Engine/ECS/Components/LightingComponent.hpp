#ifndef LIGHTINGCOMPONENT_H_
#define LIGHTINGCOMPONENT_H_
#include <glm/glm.hpp>

#include "Component.hpp"
#include "Types/LightTypes.hpp"

class LightingComponent : public Component {
public:
  enum TYPE {
    NONE,
    POINT,
    DIRECTIONAL,
  };

  LightingComponent(BaseLight *light, TYPE type);
  virtual ~LightingComponent();

  TYPE getType() { return m_t; };
  BaseLight *getBaseLight() { return light; };

private:
  TYPE m_t{NONE};
  BaseLight *light;
};
#endif // LIGHTINGCOMPONENT_H_
