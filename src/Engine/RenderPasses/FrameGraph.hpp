#ifndef FRAMEGRAPH_H_
#define FRAMEGRAPH_H_

#include "Camera.hpp"
#include <RenderPasses/RenderPass.hpp>
#include <ShaderPrograms/QuadShaderProgram.hpp>
#include <vector>

class FrameGraph {
public:
  FrameGraph() = delete;
  FrameGraph(Camera &cam);
  virtual ~FrameGraph(){};

  void draw(ECSManager &eManager);
  void setViewport(unsigned int w, unsigned int h);

private:
  Camera &m_camera;
  unsigned int m_width{800}, m_height{800};
  unsigned int quadVAO;
  unsigned int gBuffer;
  unsigned int gPosition, gNormal, gAlbedo, gEmissive;
  unsigned int rboDepth;
  QuadShaderProgram m_quadShader;
  std::vector<RenderPass *> m_renderPass;
};

#endif // FRAMEGRAPH_H_
