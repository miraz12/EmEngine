#include "Point.hpp"
#include <Graphics/RenderResources.hpp>

Point::Point(float x, float y, float z)
{
  m_vertices = { x, y, z };
  glm::mat4 modelMat = glm::identity<glm::mat4>();
  newNode(modelMat);

  p_numMeshes = 1;
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  p_meshes[0].numPrims = 1;
  p_meshes[0].m_primitives = std::make_unique<Primitive[]>(1);
  Primitive* newPrim = &p_meshes[0].m_primitives[0];

  // Use abstracted geometry creation
  auto& resources = gfx::RenderResources::getInstance();

  std::array<gfx::VertexBinding, 1> bindings = {
    { { 0, 3 * sizeof(float), false } }
  };
  std::array<gfx::VertexAttribute, 1> attributes = { {
    { 0, 0, 0, gfx::PixelFormat::RGB32F } // POSITION at location 0
  } };

  auto result = resources.createGeometry(m_vertices.data(),
                                         m_vertices.size() * sizeof(float),
                                         nullptr,
                                         0,
                                         bindings,
                                         attributes,
                                         gfx::PrimitiveTopology::Points,
                                         1,
                                         "Point");

  newPrim->m_vaoId = result.vao;
  newPrim->m_vboId = result.vbo;
  newPrim->m_topology = gfx::PrimitiveTopology::Points;
  newPrim->m_count = 1;
  newPrim->m_offset = 0;
}
