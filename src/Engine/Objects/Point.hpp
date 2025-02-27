#ifndef POINT_H_
#define POINT_H_

#include "Objects/GraphicsObject.hpp"

class Point final : public GraphicsObject
{
public:
  Point(float x, float y, float z);
  ~Point() override = default;

private:
  std::array<float, 3> m_vertices;
};

#endif // POINT_H_
