#include "Quad.hpp"

#include "glm/fwd.hpp"
#include <iostream>

Quad::Quad() {
  glm::mat4 modelMat = glm::identity<glm::mat4>();
  Node *n = new Node;
  n->mesh = 0;
  n->nodeMat = modelMat;
  p_nodes.push_back(n);

  p_numMeshes = 1;
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  p_meshes[0].numPrims = 1;
  p_meshes[0].m_primitives = std::make_unique<Primitive[]>(1);
  Primitive *newPrim = &p_meshes[0].m_primitives[0];
  uint32_t vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  newPrim->m_vao = vao;
  newPrim->m_mode = GL_TRIANGLES;

  newPrim->m_count = 6;
  newPrim->m_type = GL_UNSIGNED_INT;
  newPrim->m_offset = 0;

  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_indices), m_indices, GL_STATIC_DRAW);
  newPrim->m_ebo = ebo;
  newPrim->m_drawType = 1;

  uint32_t vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  Primitive::AttribInfo attribInfo;
  attribInfo.vbo = 0;
  attribInfo.type = 3;
  attribInfo.componentType = GL_FLOAT;
  attribInfo.normalized = GL_FALSE;
  attribInfo.byteStride = 9 * sizeof(float);
  attribInfo.byteOffset = 0;
  newPrim->attributes["POSITION"] = attribInfo;

  // glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 *
  // sizeof(float))); glEnableVertexAttribArray(1); attribInfo.vbo = 1; attribInfo.type = 4;
  // attribInfo.componentType = GL_FLOAT;
  // attribInfo.normalized = GL_FALSE;
  // attribInfo.byteStride = 9 * sizeof(float);
  // attribInfo.byteOffset = 3 * sizeof(float);
  // newPrim->attributes["NORMAL"] = attribInfo;

  // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(7 *
  // sizeof(float))); glEnableVertexAttribArray(2); attribInfo.vbo = 2; attribInfo.type = 2;
  // attribInfo.componentType = GL_FLOAT;
  // attribInfo.normalized = GL_FALSE;
  // attribInfo.byteStride = 9 * sizeof(float);
  // attribInfo.byteOffset = 7 * sizeof(float);
  // newPrim->attributes["TEXCOORD_0"] = attribInfo;
  glBindVertexArray(0);
}
