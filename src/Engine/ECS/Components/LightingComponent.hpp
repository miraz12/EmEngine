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

  LightingComponent(std::shared_ptr<BaseLight> light, TYPE type);
  virtual ~LightingComponent() = default;

  TYPE getType() { return m_t; };
  BaseLight &getBaseLight() { return *light.get(); };

private:
  TYPE m_t{NONE};
  std::shared_ptr<BaseLight> light;
};
#endif // LIGHTINGCOMPONENT_H_
