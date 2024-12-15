#ifndef NODE_H_
#define NODE_H_

class Node
{
public:
  Node() = default;
  ~Node() = default;

  std::string name{ "none" };
  i32 mesh{ -1 };
  i32 skin{ -1 };
  i32 parent{ -1 };
  std::vector<i32> children;
  glm::mat4 nodeMat{ glm::identity<glm::mat4>() };
  glm::mat4 animMat{ glm::identity<glm::mat4>() };
  glm::vec3 scale{ 1.0f };
  glm::quat rot{ 1.0f, 0.0f, 0.0f, 0.0f };
  glm::vec3 trans{ 0.0f };
};

#endif // NODE_H_
