#ifndef MESHOBJ_H_
#define MESHOBJ_H_

#include "Primitive.hpp"
#include <ShaderPrograms/ShaderProgram.hpp>

class Mesh
{
public:
  Mesh() = default;
  virtual ~Mesh() = default;
  u32 numPrims{ 0 };
  std::unique_ptr<Primitive[]> m_primitives;
};

#endif // MESHOBJ_H_
