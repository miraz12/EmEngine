#ifndef CUBE_H_
#define CUBE_H_

#include "GraphicsObject.hpp"

class Cube final : public GraphicsObject {
public:
  Cube();
  ~Cube() override = default;
};

#endif // CUBE_H_
