#ifndef FRAMEGRAPH_H_
#define FRAMEGRAPH_H_

#include "Camera.hpp"
#include <RenderPasses/RenderPass.hpp>
#include <ShaderPrograms/QuadShaderProgram.hpp>
#include <vector>

class FrameGraph {
public:
  FrameGraph();
  virtual ~FrameGraph(){};

  void draw(ECSManager &eManager);
  void setViewport(unsigned int w, unsigned int h);

private:
  unsigned int m_width{800}, m_height{800};
  QuadShaderProgram m_quadShader;
  std::vector<RenderPass *> m_renderPass;
};

#endif // FRAMEGRAPH_H_
