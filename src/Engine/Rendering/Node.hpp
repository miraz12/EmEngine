#ifndef NODE_H_
#define NODE_H_
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

class Node {
public:
  Node() = default;
  ~Node() = default;

  uint32_t mesh{0};
  glm::mat4 nodeMat{glm::identity<glm::mat4>()};
};

#endif // NODE_H_
