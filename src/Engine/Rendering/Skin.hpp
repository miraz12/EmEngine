#ifndef SKIN_H_
#define SKIN_H_

class Skin
{
public:
  std::string name;
  u32 skeletonRoot = 0;
  std::vector<glm::mat4> inverseBindMatrices;
  std::vector<i32> joints;
};

#endif // SKIN_H_
