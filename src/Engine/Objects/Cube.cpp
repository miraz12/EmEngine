#include "Cube.hpp"
#include <Graphics/RenderResources.hpp>

Cube::Cube()
{
  // Format: position(3) + normal(3) + texcoord(2) = 8 floats per vertex
  float vertices[] = { // clang-format off
    // Front face (facing +Z) - CCW from outside
    // Position              Normal              TexCoord
    -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

    // Back face (facing -Z) - CCW from outside
     0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

    // Left face (facing -X) - CCW from outside
    -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

    // Right face (facing +X) - CCW from outside
     0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,    1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,    1.0f,  0.0f,  0.0f,  0.0f, 0.0f,

    // Top face (facing +Y) - CCW from outside
    -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,  0.0f, 0.0f,

    // Bottom face (facing -Y) - CCW from outside
    -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,  0.0f, 0.0f
  };
  // clang-format on

  glm::mat4 modelMat = glm::identity<glm::mat4>();
  newNode(modelMat);

  p_numMeshes = 1;
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  p_meshes[0].numPrims = 1;
  p_meshes[0].m_primitives = std::make_unique<Primitive[]>(1);
  Primitive* newPrim = &p_meshes[0].m_primitives[0];

  // Use abstracted geometry creation
  auto& resources = gfx::RenderResources::getInstance();

  // Stride is 8 floats: 3 for position + 3 for normal + 2 for texcoord
  constexpr u32 stride = 8 * sizeof(float);

  std::array<gfx::VertexBinding, 1> bindings = { { { 0, stride, false } } };
  std::array<gfx::VertexAttribute, 3> attributes = { {
    { 0, 0, 0, gfx::PixelFormat::RGB32F }, // POSITION at location 0
    { 1,
      0,
      3 * sizeof(float),
      gfx::PixelFormat::RGB32F }, // NORMAL at location 1
    { 3,
      0,
      6 * sizeof(float),
      gfx::PixelFormat::RG32F } // TEXCOORD_0 at location 3
  } };

  auto result = resources.createGeometry(vertices,
                                         sizeof(vertices),
                                         nullptr,
                                         0,
                                         bindings,
                                         attributes,
                                         gfx::PrimitiveTopology::Triangles,
                                         36,
                                         "Cube");

  newPrim->m_vaoId = result.vao;
  newPrim->m_vboId = result.vbo;
  newPrim->m_topology = gfx::PrimitiveTopology::Triangles;
  newPrim->m_count = 36;
  newPrim->m_offset = 0;

  p_coll = new btBoxShape(btVector3(0.5, 0.5, 0.5));

  defaultMat.m_metallicFactor = 0.0f;
  defaultMat.m_roughnessFactor = 1.0f;
}
