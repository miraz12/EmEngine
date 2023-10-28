#ifndef SHAPE_H_
#define SHAPE_H_

#include <vector>

class Shape {
public:
  Shape();
  virtual ~Shape();

  void addVertex(glm::vec2 vertex);
  void addNormal(glm::vec2 normal);

  void clearVertices();
  void clearNormals();

  void setTransformMatrix(const glm::mat4 &matrix);

  const std::vector<glm::vec2> &getTransformedVertices();
  const std::vector<glm::vec2> &getTransformedNormals();

private:
  std::vector<glm::vec2> m_originalVertices;
  std::vector<glm::vec2> m_originalNormals;

  std::vector<glm::vec2> m_transformedVertices;
  std::vector<glm::vec2> m_transformedNormals;

  bool m_verticesNeedsUpdate;
  bool m_normalsNeedsUpdate;

  glm::mat4 m_transformMatrix;
};
#endif // SHAPE_H_
