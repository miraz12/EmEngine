#ifndef QUAD_H_
#define QUAD_H_

#include "GraphicsObject.hpp"

class Quad : public GraphicsObject {
public:
  Quad();
  virtual ~Quad() = default;

private:
  float m_vertices[36] = {
      -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.0f, 1.0f,
      0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 0.0f,
      0.5f,  0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.5f, 1.0f, 0.0f,
  };

  u32 m_indices[6] = {
      0, 1, 2, 2, 1, 3,
  };
};
#endif // QUAD_H_
