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
  LightingComponent();
  virtual ~LightingComponent();

  void SetupPointLight(glm::vec3 color, float ambient, float diffuse,
                       float constant, float linear, float quadratic,
                       glm::vec3 pos);
  void SetupDirectionalLight(glm::vec3 color, float ambient, float diffuse,
                             glm::vec3 dir);
  TYPE getType() { return m_t; };
  BaseLight *getBaseLight() { return light; };

 private:
  TYPE m_t{NONE};
  BaseLight *light;
};
#endif  // LIGHTINGCOMPONENT_H_
