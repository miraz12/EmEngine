#ifndef LINE_H_
#define LINE_H_

#include "GraphicsObject.hpp"
class Line final : public GraphicsObject {
public:
  Line(float x1, float y1, float z1, float x2, float y2, float z2);
  ~Line() override = default;

private:
  std::array<float, 6> m_vertices;
};

#endif // LINE_H_
