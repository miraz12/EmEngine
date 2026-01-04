#include "Cube.hpp"

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

  // Stride is 8 floats: 3 for position + 3 for normal + 2 for texcoord
  GLsizei stride = 8 * sizeof(float);

  // Position attribute (location 0)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
  glEnableVertexAttribArray(0);
  Primitive::AttribInfo posAttrib;
  posAttrib.vbo = 0;
  posAttrib.type = 3;
  posAttrib.componentType = GL_FLOAT;
  posAttrib.normalized = GL_FALSE;
  posAttrib.byteStride = stride;
  posAttrib.byteOffset = 0;
  newPrim->attributes["POSITION"] = posAttrib;

  // Normal attribute (location 1)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  Primitive::AttribInfo normAttrib;
  normAttrib.vbo = 0;
  normAttrib.type = 3;
  normAttrib.componentType = GL_FLOAT;
  normAttrib.normalized = GL_FALSE;
  normAttrib.byteStride = stride;
  normAttrib.byteOffset = 3 * sizeof(float);
  newPrim->attributes["NORMAL"] = normAttrib;

  // Texture coordinate attribute (location 3)
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(3);
  Primitive::AttribInfo texAttrib;
  texAttrib.vbo = 0;
  texAttrib.type = 2;
  texAttrib.componentType = GL_FLOAT;
  texAttrib.normalized = GL_FALSE;
  texAttrib.byteStride = stride;
  texAttrib.byteOffset = 6 * sizeof(float);
  newPrim->attributes["TEXCOORD_0"] = texAttrib;

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  p_coll = new btBoxShape(btVector3(0.5, 0.5, 0.5));
}
