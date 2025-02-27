#include "GeometryPass.hpp"
#include "ECS/Components/AnimationComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include <ECS/ECSManager.hpp>
#include <ECS/Systems/CameraSystem.hpp>
#include <Managers/FrameBufferManager.hpp>
#include <RenderPasses/FrameGraph.hpp>
#include <RenderPasses/RenderPass.hpp>
#include <ShaderPrograms/ShaderProgram.hpp>

GeometryPass::GeometryPass()
  : RenderPass("resources/Shaders/meshVertex.glsl",
               "resources/Shaders/pbrMeshFragment.glsl")
{
  glGenFramebuffers(1, &gBuffer);
  glGenRenderbuffers(1, &rboDepth);
  p_fboManager.setFBO("gBuffer", gBuffer);
  setViewport(p_width, p_height);

  p_shaderProgram.setUniformBinding("modelMatrix");
  p_shaderProgram.setUniformBinding("viewMatrix");
  p_shaderProgram.setUniformBinding("projMatrix");
  p_shaderProgram.setUniformBinding("jointTransforms");

  p_shaderProgram.setUniformBinding("textures");
  p_shaderProgram.setUniformBinding("material");
  p_shaderProgram.setUniformBinding("alphaMode");
  p_shaderProgram.setUniformBinding("alphaCutoff");
  p_shaderProgram.setUniformBinding("emissiveFactor");
  p_shaderProgram.setUniformBinding("baseColorFactor");
  p_shaderProgram.setUniformBinding("roughnessFactor");
  p_shaderProgram.setUniformBinding("metallicFactor");
  p_shaderProgram.setUniformBinding("meshMatrix");
  p_shaderProgram.setUniformBinding("jointMatrices");
  p_shaderProgram.setUniformBinding("jointMats");
  p_shaderProgram.setUniformBinding("is_skinned");

  p_shaderProgram.setAttribBinding("POSITION");
  p_shaderProgram.setAttribBinding("NORMAL");
  p_shaderProgram.setAttribBinding("TANGENT");
  p_shaderProgram.setAttribBinding("TEXCOORD_0");
  p_shaderProgram.setAttribBinding("JOINTS_0");
  p_shaderProgram.setAttribBinding("WEIGHTS_0");

  u32 jointMats;
  glGenTextures(1, &jointMats);
  p_textureManager.setTexture("jointMats", jointMats);

  p_textureManager.bindTexture("jointMats");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
GeometryPass::Init(FrameGraph& fGraph)
{
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture(
    "gPositionAo");
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture(
    "gNormalMetal");
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture(
    "gAlbedoSpecRough");
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture(
    "gEmissive");
}

void
GeometryPass::Execute(ECSManager& eManager)
{
  glBindFramebuffer(GL_FRAMEBUFFER, p_fboManager.getFBO("gBuffer"));
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  p_shaderProgram.use();

  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  CameraSystem::bindProjViewMatrix(
    cam,
    p_shaderProgram.getUniformLocation("projMatrix"),
    p_shaderProgram.getUniformLocation("viewMatrix"));

  std::vector<Entity> view = eManager.view<GraphicsComponent>();
  for (auto e : view) {
    std::shared_ptr<PositionComponent> p =
      eManager.getComponent<PositionComponent>(e);
    std::shared_ptr<GraphicsComponent> g =
      eManager.getComponent<GraphicsComponent>(e);

    // Check for position component and apply its model matrix
    if (p) {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(p->model));
    } else {
      glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(glm::identity<glm::mat4>()));
    }

    g->m_grapObj->draw(p_shaderProgram);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
GeometryPass::setViewport(u32 w, u32 h)
{
  p_width = w;
  p_height = h;

  glBindFramebuffer(GL_FRAMEBUFFER, p_fboManager.getFBO("gBuffer"));

  // - position color buffer
  u32 gPosition = p_textureManager.loadTexture(
    "gPositionAo", GL_RGBA16F, GL_RGBA, GL_FLOAT, p_width, p_height, 0);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

  // - normal color buffer
  u32 gNormal = p_textureManager.loadTexture(
    "gNormalMetal", GL_RGBA16F, GL_RGBA, GL_FLOAT, p_width, p_height, 0);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

  // - color
  u32 gAlbedo = p_textureManager.loadTexture(
    "gAlbedoSpecRough", GL_RGBA16F, GL_RGBA, GL_FLOAT, p_width, p_height, 0);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

  // - emissive color buffer
  u32 gEmissive = p_textureManager.loadTexture(
    "gEmissive", GL_RGBA16F, GL_RGBA, GL_FLOAT, p_width, p_height, 0);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gEmissive, 0);

  // - tell OpenGL which color attachments we'll use (of this framebuffer) for
  // rendering
  u32 attachments[4] = { GL_COLOR_ATTACHMENT0,
                         GL_COLOR_ATTACHMENT1,
                         GL_COLOR_ATTACHMENT2,
                         GL_COLOR_ATTACHMENT3 };
  glDrawBuffers(4, attachments);
  // create and attach depth buffer (renderbuffer)
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(
    GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, p_width, p_height);
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
