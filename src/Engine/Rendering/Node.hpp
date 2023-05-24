#ifndef NODE_H_
#define NODE_H_
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

class Node {
public:
  Node() = default;
  ~Node() = default;

  unsigned int mesh{0};
  glm::mat4 nodeMat{glm::identity<glm::mat4>()};
};

#endif // NODE_H_
