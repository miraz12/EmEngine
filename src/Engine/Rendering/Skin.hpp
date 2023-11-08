#ifndef SKIN_H_
#define SKIN_H_

class Skin {
  std::string name;
  // Node *skeletonRoot = nullptr;
  std::vector<glm::mat4> inverseBindMatrices;
  // std::vector<Node *> joints;
};

#endif // SKIN_H_
