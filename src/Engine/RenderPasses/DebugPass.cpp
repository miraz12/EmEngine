#include "DebugPass.hpp"
#include <ECS/Components/DebugComponent.hpp>
#include <ECS/ECSManager.hpp>

DebugPass::DebugPass()
    : RenderPass("resources/Shaders/meshVertex.glsl", "resources/Shaders/pbrMeshFragment.glsl") {

  p_shaderProgram.setUniformBinding("modelMatrix");
  p_shaderProgram.setUniformBinding("viewMatrix");
  p_shaderProgram.setUniformBinding("projMatrix");
  p_shaderProgram.setUniformBinding("textures");
  p_shaderProgram.setUniformBinding("material");
  p_shaderProgram.setUniformBinding("alphaMode");
  p_shaderProgram.setUniformBinding("alphaCutoff");
  p_shaderProgram.setUniformBinding("emissiveFactor");
  p_shaderProgram.setUniformBinding("baseColorFactor");
  p_shaderProgram.setUniformBinding("roughnessFactor");
  p_shaderProgram.setUniformBinding("metallicFactor");
  p_shaderProgram.setUniformBinding("meshMatrix");

  p_shaderProgram.setAttribBinding("POSITION");
  p_shaderProgram.setAttribBinding("NORMAL");
  p_shaderProgram.setAttribBinding("TANGENT");
  p_shaderProgram.setAttribBinding("TEXCOORD_0");
}

void DebugPass::Execute(ECSManager &eManager) {
  eManager.getCamera().bindProjViewMatrix(p_shaderProgram.getUniformLocation("projMatrix"),
                                          p_shaderProgram.getUniformLocation("viewMatrix"));

#ifndef EMSCRIPTEN
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glEnable(GL_DEPTH_TEST);

  p_shaderProgram.use();

  std::vector<Entity> view = eManager.view<DebugComponent>();
  for (auto e : view) {
    std::shared_ptr<PositionComponent> p = eManager.getComponent<PositionComponent>(e);
    if (p) {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
                         glm::value_ptr(p->model));
    } else {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
                         glm::value_ptr(glm::identity<glm::mat4>()));
    }

    std::shared_ptr<DebugComponent> d = eManager.getComponent<DebugComponent>(e);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    d->m_grapObj->draw(p_shaderProgram);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
}
