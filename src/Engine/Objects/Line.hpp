#ifndef LINE_H_
#define LINE_H_

#include <Objects/GraphicsObject.hpp>
#include <array>
class Line : public GraphicsObject {
public:
  Line(float x1, float y1, float z1, float x2, float y2, float z2);
  ~Line() = default;

private:
  std::array<float, 6> m_vertices;
};

#endif // LINE_H_
