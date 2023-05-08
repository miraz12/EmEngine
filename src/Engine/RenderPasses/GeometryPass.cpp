#include "GeometryPass.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#include <ECS/ECSManager.hpp>
#include <Managers/FrameBufferManager.hpp>
#include <RenderPasses/RenderPass.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>

GeometryPass::GeometryPass()
    : RenderPass("resources/Shaders/meshVertex.glsl", "resources/Shaders/pbrMeshFragment.glsl") {
  glGenFramebuffers(1, &gBuffer);
  glGenRenderbuffers(1, &rboDepth);
  p_fboManager.setFBO("gBuffer", gBuffer);

  p_shaderProgram.setUniformBinding("modelMatrix");
  p_shaderProgram.setUniformBinding("viewMatrix");
  p_shaderProgram.setUniformBinding("projMatrix");
  p_shaderProgram.setUniformBinding("textures");
  p_shaderProgram.setUniformBinding("material");
  p_shaderProgram.setUniformBinding("emissiveFactor");
  p_shaderProgram.setUniformBinding("baseColorFactor");
  p_shaderProgram.setUniformBinding("roughnessFactor");
  p_shaderProgram.setUniformBinding("metallicFactor");

  p_shaderProgram.setAttribBinding("POSITION");
  p_shaderProgram.setAttribBinding("NORMAL");
  p_shaderProgram.setAttribBinding("TANGENT");
  p_shaderProgram.setAttribBinding("TEXCOORD_0");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GeometryPass::Execute(ECSManager &eManager) {
  glBindFramebuffer(GL_FRAMEBUFFER, p_fboManager.getFBO("gBuffer"));
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  p_shaderProgram.use();

  eManager.getCamera().bindProjViewMatrix(p_shaderProgram.getUniformLocation("projMatrix"),
                                          p_shaderProgram.getUniformLocation("viewMatrix"));

  // glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
  // glm::value_ptr(model));
  GLint tex[5] = {0, 1, 2, 3, 4};
  glUniform1iv(p_shaderProgram.getUniformLocation("textures"), 5, tex);

  std::vector<Entity> view = eManager.view<GraphicsComponent, PositionComponent>();
  for (auto e : view) {
    // PositionComponent *p = eManager.getComponent<PositionComponent>(e);
    // glm::mat4 model = p->calculateMatrix();
    GraphicsComponent *g = eManager.getComponent<GraphicsComponent>(e);
    g->grapObj->draw(p_shaderProgram);
  }
}

void GeometryPass::setViewport(unsigned int w, unsigned int h) {
  p_width = w;
  p_height = h;

}
