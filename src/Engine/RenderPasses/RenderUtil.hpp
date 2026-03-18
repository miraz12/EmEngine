#ifndef RENDERUTIL_H_
#define RENDERUTIL_H_

namespace Util {

// Quad vertex layout constants
namespace {
constexpr u32 QUAD_POSITION_COMPONENTS = 3;
constexpr u32 QUAD_TEXCOORD_COMPONENTS = 2;
constexpr u32 QUAD_STRIDE = (QUAD_POSITION_COMPONENTS + QUAD_TEXCOORD_COMPONENTS) * sizeof(float);
constexpr u32 QUAD_TEXCOORD_OFFSET = QUAD_POSITION_COMPONENTS * sizeof(float);
}

inline void
renderQuad()
{
  static u32 quadVAO = 0;
  static u32 quadVBO = 0;
  if (quadVAO == 0) {
    // clang-format off
    float quadVertices[] = {
        // positions         // texture Coords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
    };
    // clang-format on

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(
      GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, QUAD_POSITION_COMPONENTS, GL_FLOAT, GL_FALSE, QUAD_STRIDE, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
      1, QUAD_TEXCOORD_COMPONENTS, GL_FLOAT, GL_FALSE, QUAD_STRIDE, reinterpret_cast<void*>(QUAD_TEXCOORD_OFFSET));
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// Cube vertex layout constants
namespace {
constexpr u32 CUBE_POSITION_COMPONENTS = 3;
constexpr u32 CUBE_NORMAL_COMPONENTS = 3;
constexpr u32 CUBE_TEXCOORD_COMPONENTS = 2;
constexpr u32 CUBE_STRIDE = (CUBE_POSITION_COMPONENTS + CUBE_NORMAL_COMPONENTS + CUBE_TEXCOORD_COMPONENTS) * sizeof(float);
constexpr u32 CUBE_NORMAL_OFFSET = CUBE_POSITION_COMPONENTS * sizeof(float);
constexpr u32 CUBE_TEXCOORD_OFFSET = (CUBE_POSITION_COMPONENTS + CUBE_NORMAL_COMPONENTS) * sizeof(float);
constexpr u32 CUBE_VERTEX_COUNT = 36;
}

inline void
renderCube()
{
  static u32 cubeVAO{ 0 };
  static u32 cubeVBO{ 0 };
  if (cubeVAO == 0) {
    // clang-format off
      float vertices[] = {
          // back face
          -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
           1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
           1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
           1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
          -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
          -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
          // front face
          -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
           1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
           1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
           1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
          -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
          -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
          // left face
          -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
          -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
          -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
          -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
          -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
          -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
          // right face
           1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
           1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
           1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
           1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
           1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
           1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
          // bottom face
          -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
           1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
           1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
           1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
          -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
          -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
          // top face
          -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
           1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
           1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
           1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
          -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f, // bottom-left
          -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f  // top-left
      };
    // clang-format on

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, CUBE_POSITION_COMPONENTS, GL_FLOAT, GL_FALSE, CUBE_STRIDE, nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
      1, CUBE_NORMAL_COMPONENTS, GL_FLOAT, GL_FALSE, CUBE_STRIDE, reinterpret_cast<void*>(CUBE_NORMAL_OFFSET));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
      2, CUBE_TEXCOORD_COMPONENTS, GL_FLOAT, GL_FALSE, CUBE_STRIDE, reinterpret_cast<void*>(CUBE_TEXCOORD_OFFSET));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
  glBindVertexArray(cubeVAO);
  glDrawArrays(GL_TRIANGLES, 0, CUBE_VERTEX_COUNT);
}

} // namespace Util

#endif // RENDERUTIL_H_
