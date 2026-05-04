#include "DebugDrawer.hpp"

#include <Graphics/RenderResources.hpp>

DebugDrawer::DebugDrawer()
{
  lines.reserve(1000);
}

DebugDrawer::~DebugDrawer()
{
  shutdown();
}

void
DebugDrawer::initialize()
{
#ifndef EMSCRIPTEN
  if (m_initialized) {
    return;
  }

  auto& device = gfx::GraphicsDevice::getInstance();

  gfx::BufferCreateInfo vboInfo{};
  vboInfo.size = kInitialCapacity;
  vboInfo.usage = gfx::BufferUsage::Vertex;
  vboInfo.initialData = nullptr;
  m_vbo = device.createBuffer(vboInfo);
  m_vboCapacity = kInitialCapacity;

  constexpr u32 kStride = 6 * sizeof(float);

  std::array<gfx::VertexBinding, 1> bindings = { { { 0, kStride, false } } };
  std::array<gfx::VertexAttribute, 2> attributes = {
    { { 0, 0, 0, gfx::PixelFormat::RGB32F },
      { 1, 0, 3 * sizeof(float), gfx::PixelFormat::RGB32F } }
  };

  gfx::VertexArrayCreateInfo vaoInfo{};
  vaoInfo.vertexBindings = bindings;
  vaoInfo.vertexAttributes = attributes;
  vaoInfo.topology = gfx::PrimitiveTopology::Lines;
  vaoInfo.vertexCount = 0;
  vaoInfo.debugName = "DebugDrawer";

  m_vao = device.createVertexArray(vaoInfo);

  m_initialized = true;
#endif
}

void
DebugDrawer::shutdown()
{
#ifndef EMSCRIPTEN
  if (!m_initialized) {
    return;
  }

  auto& device = gfx::GraphicsDevice::getInstance();

  if (device.isValid(m_vao)) {
    device.destroyVertexArray(m_vao);
    m_vao = {};
  }
  if (device.isValid(m_vbo)) {
    device.destroyBuffer(m_vbo);
    m_vbo = {};
  }

  m_vboCapacity = 0;
  m_initialized = false;
#endif
}

void
DebugDrawer::drawLine(const glm::vec3& from,
                      const glm::vec3& to,
                      const glm::vec3& color)
{
#ifndef EMSCRIPTEN
  if (!std::isnan(from.x) && !std::isnan(to.x)) {
    lines.push_back({ from, to, color });
  }
#endif
}

void
DebugDrawer::renderAndFlush(gfx::ShaderId shader)
{
#ifndef EMSCRIPTEN
  const size_t numLines = lines.size();

  if (numLines == 0) {
    return;
  }

  if (!m_initialized) {
    initialize();
  }

  auto& device = gfx::GraphicsDevice::getInstance();
  auto& resources = gfx::RenderResources::getInstance();

  if (shader.isValid()) {
    device.bindShaderProgram(shader);
    glm::mat4 identity(1.0f);
    i32 modelMatrixLoc = device.getUniformLocation(shader, "modelMatrix");
    device.setUniformMat4(modelMatrixLoc, identity);
  }

  std::vector<float> vertexData;
  vertexData.reserve(numLines * 2 * 6);

  for (size_t i = 0; i < numLines; i++) {
    const Line& line = lines[i];

    if (std::isnan(line.from.x) || std::isnan(line.to.x)) {
      continue;
    }

    vertexData.push_back(line.from.x);
    vertexData.push_back(line.from.y);
    vertexData.push_back(line.from.z);
    vertexData.push_back(line.color.x);
    vertexData.push_back(line.color.y);
    vertexData.push_back(line.color.z);

    vertexData.push_back(line.to.x);
    vertexData.push_back(line.to.y);
    vertexData.push_back(line.to.z);
    vertexData.push_back(line.color.x);
    vertexData.push_back(line.color.y);
    vertexData.push_back(line.color.z);
  }

  if (vertexData.empty()) {
    lines.clear();
    return;
  }

  resources.setDepthTest(false);
  resources.bindDefaultFramebuffer();
  resources.setBlendEnabled(true);
  resources.setBlendFunc(gfx::BlendFactor::SrcAlpha,
                         gfx::BlendFactor::OneMinusSrcAlpha);

  u64 requiredSize = vertexData.size() * sizeof(float);

  if (requiredSize > m_vboCapacity) {
    device.destroyBuffer(m_vbo);
    u64 newCapacity = std::max(requiredSize, m_vboCapacity * 2);
    gfx::BufferCreateInfo vboInfo{};
    vboInfo.size = newCapacity;
    vboInfo.usage = gfx::BufferUsage::Vertex;
    vboInfo.initialData = nullptr;
    m_vbo = device.createBuffer(vboInfo);
    m_vboCapacity = newCapacity;
  }

  device.updateBuffer(m_vbo, 0, vertexData.data(), requiredSize);
  resources.setLineWidth(1.0f);

  u32 vertexCount = static_cast<u32>(vertexData.size() / 6);
  device.drawVertexArrayDynamic(
    m_vao, m_vbo, gfx::PrimitiveTopology::Lines, vertexCount);

  lines.clear();
#endif
}
