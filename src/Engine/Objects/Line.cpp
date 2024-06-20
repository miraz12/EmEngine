#include "Line.hpp"

Line::Line(float x1, float y1, float z1, float x2, float y2, float z2) {
  m_vertices = {x1, y1, z1, x2, y2, z2};
  glm::mat4 modelMat = glm::identity<glm::mat4>();
  newNode(modelMat);

  p_numMeshes = 1;
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  p_meshes[0].numPrims = 1;
  p_meshes[0].m_primitives = std::make_unique<Primitive[]>(1);
  Primitive *newPrim = &p_meshes[0].m_primitives[0];
  u32 vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  newPrim->m_vao = vao;
  newPrim->m_mode = GL_LINES;

  newPrim->m_count = 2;
  newPrim->m_type = GL_UNSIGNED_INT;
  newPrim->m_offset = 0;

  u32 vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  Primitive::AttribInfo attribInfo;
  attribInfo.vbo = 0;
  attribInfo.type = 3;
  attribInfo.componentType = GL_FLOAT;
  attribInfo.normalized = GL_FALSE;
  attribInfo.byteStride = 9 * sizeof(float);
  attribInfo.byteOffset = 0;
  newPrim->attributes["POSITION"] = attribInfo;

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
