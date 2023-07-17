#include "GeometryPass.hpp"

#include <ECS/ECSManager.hpp>
#include <Managers/FrameBufferManager.hpp>
#include <RenderPasses/FrameGraph.hpp>
#include <RenderPasses/RenderPass.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>

GeometryPass::GeometryPass()
    : RenderPass("resources/Shaders/meshVertex.glsl", "resources/Shaders/pbrMeshFragment.glsl") {
  glGenFramebuffers(1, &gBuffer);
  glGenRenderbuffers(1, &rboDepth);
  p_fboManager.setFBO("gBuffer", gBuffer);
  setViewport(p_width, p_height);

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

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GeometryPass::Init(FrameGraph &fGraph) {
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture("gPositionAo");
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture("gNormalMetal");
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture("gAlbedoSpecRough");
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture("gEmissive");
}

void GeometryPass::Execute(ECSManager &eManager) {
  glBindFramebuffer(GL_FRAMEBUFFER, p_fboManager.getFBO("gBuffer"));
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  p_shaderProgram.use();

  eManager.getCamera().bindProjViewMatrix(p_shaderProgram.getUniformLocation("projMatrix"),
                                          p_shaderProgram.getUniformLocation("viewMatrix"));

  std::vector<Entity> view = eManager.view<GraphicsComponent>();
  for (auto e : view) {
    std::shared_ptr<PositionComponent> p = eManager.getComponent<PositionComponent>(e);
    if (p) {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
                         glm::value_ptr(p->model));
    } else {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
                         glm::value_ptr(glm::identity<glm::mat4>()));
    }

    std::shared_ptr<GraphicsComponent> g = eManager.getComponent<GraphicsComponent>(e);
    g->m_grapObj.draw(p_shaderProgram);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GeometryPass::setViewport(uint32_t w, uint32_t h) {
  p_width = w;
  p_height = h;

  glBindFramebuffer(GL_FRAMEBUFFER, p_fboManager.getFBO("gBuffer"));

  // - position color buffer
  uint32_t gPosition = p_textureManager.loadTexture("gPositionAo", GL_RGBA16F, GL_RGBA, GL_FLOAT,
                                                    p_width, p_height, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

  // - normal color buffer
  uint32_t gNormal = p_textureManager.loadTexture("gNormalMetal", GL_RGBA16F, GL_RGBA, GL_FLOAT,
                                                  p_width, p_height, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

  // - color
  uint32_t gAlbedo = p_textureManager.loadTexture("gAlbedoSpecRough", GL_RGBA16F, GL_RGBA, GL_FLOAT,
                                                  p_width, p_height, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

  // - emissive color buffer
  uint32_t gEmissive = p_textureManager.loadTexture("gEmissive", GL_RGBA16F, GL_RGBA, GL_FLOAT,
                                                    p_width, p_height, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gEmissive, 0);

  // - tell OpenGL which color attachments we'll use (of this framebuffer) for
  // rendering
  uint32_t attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                             GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, attachments);
  // create and attach depth buffer (renderbuffer)
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, p_width, p_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
