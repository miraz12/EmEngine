#ifndef LIGHTTYPES_H_
#define LIGHTTYPES_H_

struct BaseLight
{
  glm::vec3 color = glm::vec3(0.0f);
};
struct DirectionalLight : public BaseLight
{
  glm::vec3 direction = glm::vec3(0.0);
  float ambientIntensity = 0.0f;
};
struct PointLight : public BaseLight
{
  glm::vec3 position = glm::vec3(0.0);
  float constant;
  float linear;
  float quadratic;
};

#endif // LIGHTTYPES_H_
