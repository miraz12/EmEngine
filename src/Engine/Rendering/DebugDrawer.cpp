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

  // Create persistent VBO with initial capacity
  gfx::BufferCreateInfo vboInfo{};
  vboInfo.size = kInitialCapacity;
  vboInfo.usage = gfx::BufferUsage::Vertex;
  vboInfo.initialData = nullptr; // Will be filled each frame
  m_vbo = device.createBuffer(vboInfo);
  m_vboCapacity = kInitialCapacity;

  // Create VAO with vertex layout: position (vec3) + color (vec3)
  constexpr u32 kStride = 6 * sizeof(float);

  std::array<gfx::VertexBinding, 1> bindings = { { { 0, kStride, false } } };
  std::array<gfx::VertexAttribute, 2> attributes = { {
    { 0, 0, 0, gfx::PixelFormat::RGB32F }, // POSITION at location 0
    { 1, 0, 3 * sizeof(float), gfx::PixelFormat::RGB32F } // COLOR at location 1
  } };

  gfx::VertexArrayCreateInfo vaoInfo{};
  vaoInfo.vertexBindings = bindings;
  vaoInfo.vertexAttributes = attributes;
  vaoInfo.topology = gfx::PrimitiveTopology::Lines;
  vaoInfo.vertexCount = 0; // Will be set per frame
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

#ifdef EMSCRIPTEN
void
DebugDrawer::drawLine(const btVector3& from,
                      const btVector3& to,
                      const btVector3& color)
{
  // No implementation for Emscripten
}
#else
void
DebugDrawer::drawLine(const btVector3& from,
                      const btVector3& to,
                      const btVector3& color)
{
  // Only add the line if it's valid and we haven't exceeded the maximum
  if (!std::isnan(from.x()) && !std::isnan(from.y()) && !std::isnan(from.z()) &&
      !std::isnan(to.x()) && !std::isnan(to.y()) && !std::isnan(to.z())) {

    DebugDrawer::Line l;
    l.from = from;
    l.to = to;
    l.color = color;
    lines.push_back(l);
  }
}
#endif

void
DebugDrawer::drawContactPoint(const btVector3& /* pointOnB */,
                              const btVector3& /* normalOnB */,
                              btScalar /* distance */,
                              i32 /* lifeTime */,
                              const btVector3& /* color */)
{
  // Implement if needed
}

void
DebugDrawer::reportErrorWarning(const char* /* warningString */)
{
  // Implement if needed
}

void
DebugDrawer::draw3dText(const btVector3& /* location */,
                        const char* /* textString */)
{
  // Implement if needed
}

void
DebugDrawer::renderAndFlush(gfx::ShaderId shader)
{
#ifndef EMSCRIPTEN
  const size_t numLines = lines.size();

  if (numLines == 0) {
    return;
  }

  // Lazy initialization if not done yet
  if (!m_initialized) {
    initialize();
  }

  auto& device = gfx::GraphicsDevice::getInstance();
  auto& resources = gfx::RenderResources::getInstance();

  // Render only physics debug lines in 3D mode
  resources.setDepthTest(true);

  // Set up the shader program
  if (shader.isValid()) {
    // Use the external shader provided by DebugPass
    device.bindShaderProgram(shader);

    // Set the model matrix to identity (viewMatrix/projMatrix come from
    // CameraData UBO)
    glm::mat4 identity(1.0f);
    i32 modelMatrixLoc = device.getUniformLocation(shader, "modelMatrix");
    device.setUniformMat4(modelMatrixLoc, identity);
    // Note: viewMatrix and projMatrix now come from CameraData UBO,
    // which should already be updated by the main render passes
  }
  // Note: Legacy fixed-function fallback removed - shader is required

  // Create arrays for our vertex data
  std::vector<float> vertexData;
  vertexData.reserve(numLines * 2 * 6);

  // Fill the buffers with data from all lines
  for (size_t i = 0; i < numLines; i++) {
    const Line& line = lines[i];

    // Skip any invalid lines
    if (std::isnan(line.from.x()) || std::isnan(line.to.x())) {
      continue;
    }

    // First vertex (from)
    vertexData.push_back(line.from.x());
    vertexData.push_back(line.from.y());
    vertexData.push_back(line.from.z());
    vertexData.push_back(line.color.x());
    vertexData.push_back(line.color.y());
    vertexData.push_back(line.color.z());

    // Second vertex (to)
    vertexData.push_back(line.to.x());
    vertexData.push_back(line.to.y());
    vertexData.push_back(line.to.z());
    vertexData.push_back(line.color.x());
    vertexData.push_back(line.color.y());
    vertexData.push_back(line.color.z());
  }

  // Skip rendering if we have no valid vertex data
  if (vertexData.empty()) {
    lines.clear();
    return;
  }

  // For maximum visibility, we disable depth testing
  resources.setDepthTest(false);

  // Make sure we're drawing to the default framebuffer (screen)
  resources.bindDefaultFramebuffer();

  // Ensure blending is enabled for transparency
  resources.setBlendEnabled(true);
  resources.setBlendFunc(gfx::BlendFactor::SrcAlpha,
                         gfx::BlendFactor::OneMinusSrcAlpha);

  // Calculate required buffer size
  u64 requiredSize = vertexData.size() * sizeof(float);

  // Resize buffer if needed (double capacity to reduce reallocations)
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

  // Upload vertex data to VBO (orphaning pattern for async safety)
  device.updateBuffer(m_vbo, 0, vertexData.data(), requiredSize);

  // Set line width
  resources.setLineWidth(1.0f);

  // Draw using the abstracted API
  u32 vertexCount = static_cast<u32>(vertexData.size() / 6);
  device.drawVertexArrayDynamic(
    m_vao, m_vbo, gfx::PrimitiveTopology::Lines, vertexCount);

  // Clear the lines vector for next frame
  lines.clear();
#endif
}
