#include "GraphicsObject.hpp"

GraphicsObject::~GraphicsObject() {
  for (auto &n : p_nodes) {
      delete n;
  }
}
