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
  : RenderPass("resources/Shaders/mesh.vert", "resources/Shaders/pbrMesh.frag")
{
  glGenFramebuffers(1, &gBuffer);
  glGenRenderbuffers(1, &rboDepth);
  m_fboManager.setFBO("gBuffer", gBuffer);
  setViewport(m_width, m_height);

  m_shaderProgram.setUniformBinding("modelMatrix");
  m_shaderProgram.setUniformBinding("viewMatrix");
  m_shaderProgram.setUniformBinding("projMatrix");
  m_shaderProgram.setUniformBinding("jointTransforms");

  m_shaderProgram.setUniformBinding("textures");
  m_shaderProgram.setUniformBinding("material");
  m_shaderProgram.setUniformBinding("alphaMode");
  m_shaderProgram.setUniformBinding("alphaCutoff");
  m_shaderProgram.setUniformBinding("emissiveFactor");
  m_shaderProgram.setUniformBinding("baseColorFactor");
  m_shaderProgram.setUniformBinding("roughnessFactor");
  m_shaderProgram.setUniformBinding("metallicFactor");
  m_shaderProgram.setUniformBinding("meshMatrix");
  m_shaderProgram.setUniformBinding("jointMats");
  m_shaderProgram.setUniformBinding("is_skinned");

  m_shaderProgram.setAttribBinding("POSITION");
  m_shaderProgram.setAttribBinding("NORMAL");
  m_shaderProgram.setAttribBinding("TANGENT");
  m_shaderProgram.setAttribBinding("TEXCOORD_0");
  m_shaderProgram.setAttribBinding("JOINTS_0");
  m_shaderProgram.setAttribBinding("WEIGHTS_0");

  u32 jointMats;
  glGenTextures(1, &jointMats);
  m_textureManager.setTexture("jointMats", jointMats);

  m_textureManager.bindTexture("jointMats");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
GeometryPass::Init(FrameGraph& fGraph)
{
  fGraph.getPass(PassId::kLight)->addTexture("gPositionAo");
  fGraph.getPass(PassId::kLight)->addTexture("gNormalMetal");
  fGraph.getPass(PassId::kLight)->addTexture("gAlbedoSpecRough");
  fGraph.getPass(PassId::kLight)->addTexture("gEmissive");
}

void
GeometryPass::Execute(ECSManager& eManager)
{
#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Geometry Pass");
#endif
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboManager.getFBO("gBuffer"));
  glViewport(0, 0, m_width, m_height);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDepthMask(GL_TRUE);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  m_shaderProgram.use();

  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  CameraSystem::bindProjViewMatrix(
    cam,
    m_shaderProgram.getUniformLocation("projMatrix"),
    m_shaderProgram.getUniformLocation("viewMatrix"));

  std::vector<Entity> view = eManager.view<GraphicsComponent>();
  for (auto e : view) {
    std::shared_ptr<PositionComponent> p =
      eManager.getComponent<PositionComponent>(e);
    std::shared_ptr<GraphicsComponent> g =
      eManager.getComponent<GraphicsComponent>(e);

    // Check for position component and apply its model matrix
    if (p) {
      glUniformMatrix4fv(m_shaderProgram.getUniformLocation("modelMatrix"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(p->model));
    } else {
      glUniformMatrix4fv(m_shaderProgram.getUniformLocation("modelMatrix"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(glm::identity<glm::mat4>()));
    }

    g->m_grapObj->draw(m_shaderProgram);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPopDebugGroup();
#endif
}

void
GeometryPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  glBindFramebuffer(GL_FRAMEBUFFER, m_fboManager.getFBO("gBuffer"));

  // - position color buffer
  u32 gPosition = m_textureManager.loadTexture(
    "gPositionAo", GL_RGBA16F, GL_RGBA, GL_FLOAT, m_width, m_height, 0);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

  // - normal color buffer
  u32 gNormal = m_textureManager.loadTexture(
    "gNormalMetal", GL_RGBA16F, GL_RGBA, GL_FLOAT, m_width, m_height, 0);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

  // - color
  u32 gAlbedo = m_textureManager.loadTexture(
    "gAlbedoSpecRough", GL_RGBA16F, GL_RGBA, GL_FLOAT, m_width, m_height, 0);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

  // - emissive color buffer
  u32 gEmissive = m_textureManager.loadTexture(
    "gEmissive", GL_RGBA16F, GL_RGBA, GL_FLOAT, m_width, m_height, 0);
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
    GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
