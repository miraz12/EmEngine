#ifndef NODE_H_
#define NODE_H_

class Node {
public:
  Node() = default;
  ~Node() = default;

  u32 mesh{0};
  glm::mat4 nodeMat{glm::identity<glm::mat4>()};
};

#endif // NODE_H_
