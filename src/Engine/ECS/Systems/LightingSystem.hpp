#ifndef LIGHTINGSYSTEM_H_
#define LIGHTINGSYSTEM_H_

#include "Camera.hpp"
#include "ShaderPrograms/LightingShaderProgram.hpp"
#include "System.hpp"
#include "Types/LightTypes.hpp"

class LightingSystem : public System {
 public:
  LightingSystem(ECSManager *ECSManager, Camera &cam);
  void update(float dt);
  void setViewport(unsigned int w, unsigned int h);

 private:
  void initGL();

  Camera &m_camera;
  unsigned int m_width{800}, m_height{800};
  unsigned int quadVAO;
  unsigned int gBuffer;
  unsigned int gPosition, gNormal, gAlbedo;
  unsigned int rboDepth;
  LightingShaderProgram m_shaderProgram;
};

#endif  // LIGHTINGSYSTEM_H_
