#include "Quad.hpp"
#include <Graphics/RenderResources.hpp>

Quad::Quad()
{
  glm::mat4 modelMat = glm::identity<glm::mat4>();
  newNode(modelMat);

  p_numMeshes = 1;
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  p_meshes[0].numPrims = 1;
  p_meshes[0].m_primitives = std::make_unique<Primitive[]>(1);
  Primitive* newPrim = &p_meshes[0].m_primitives[0];

  // Use abstracted geometry creation
  auto& resources = gfx::RenderResources::getInstance();

  // Quad layout: 9 floats per vertex (position xyz, color rgb, texcoord uv +
  // unknown)
  std::array<gfx::VertexBinding, 1> bindings = {
    { { 0, 9 * sizeof(float), false } }
  };
  std::array<gfx::VertexAttribute, 1> attributes = { {
    { 0, 0, 0, gfx::PixelFormat::RGB32F } // POSITION at location 0
  } };

  auto result = resources.createGeometry(m_vertices,
                                         sizeof(m_vertices),
                                         m_indices,
                                         sizeof(m_indices),
                                         bindings,
                                         attributes,
                                         gfx::PrimitiveTopology::Triangles,
                                         4,
                                         "Quad");

  newPrim->m_vaoId = result.vao;
  newPrim->m_vboId = result.vbo;
  newPrim->m_eboId = result.ebo;
  newPrim->m_topology = gfx::PrimitiveTopology::Triangles;
  newPrim->m_indexType = gfx::IndexType::U32;
  newPrim->m_count = 6;
  newPrim->m_offset = 0;

  p_coll = new btBoxShape(btVector3(0.5, 0.5, 0.5));
}
