#include "Cube.hpp"

Cube::Cube()
{
  float vertices[] = { // clang-format off
    // Front face (facing +Z) - CCW from outside
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,

    // Back face (facing -Z) - CCW from outside
     0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,

    // Left face (facing -X) - CCW from outside
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,

    // Right face (facing +X) - CCW from outside
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,

    // Top face (facing +Y) - CCW from outside
    -0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f,  0.5f,

    // Bottom face (facing -Y) - CCW from outside
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f
  };
  // clang-format on

  glm::mat4 modelMat = glm::identity<glm::mat4>();
  newNode(modelMat);

  p_numMeshes = 1;
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  p_meshes[0].numPrims = 1;
  p_meshes[0].m_primitives = std::make_unique<Primitive[]>(1);
  Primitive* newPrim = &p_meshes[0].m_primitives[0];
  u32 vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  newPrim->m_vao = vao;
  newPrim->m_mode = GL_TRIANGLES;

  newPrim->m_count = 36;
  newPrim->m_type = GL_UNSIGNED_INT;
  newPrim->m_offset = 0;

  u32 vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

  p_coll = new btBoxShape(btVector3(0.5, 0.5, 0.5));
}
