#ifndef GRAPHICSSYSTEM_H_
#define GRAPHICSSYSTEM_H_

#include <array>

#include "../../Camera.hpp"
#include "../../ShaderPrograms/QuadShaderProgram.hpp"
#include "System.hpp"

class GraphicsSystem : public System {
 public:
  GraphicsSystem(ECSManager *ECSManager, Camera &cam);
  void update(float dt);
  void setViewport(unsigned int w, unsigned int h);

 private:
  void initGL();

  Camera &m_camera;

  // Framebuffer variables
  std::array<unsigned int, 2> m_fbos;
  std::array<unsigned int, 2> m_colTexs;
  std::array<unsigned int, 2> m_rbos;

  unsigned int m_width{800}, m_height{800};
  unsigned int quadVAO;
  unsigned int gBuffer;
  unsigned int gPosition, gNormal, gAlbedoSpec, gDepth;
  QuadShaderProgram m_shaderProgram;
};
#endif  // GRAPHICSSYSTEM_H_
