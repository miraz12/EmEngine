#ifndef HEIGHTMAP_H_
#define HEIGHTMAP_H_

#include <Objects/GraphicsObject.hpp>
class Heightmap : public GraphicsObject {
public:
  Heightmap() = delete;
  Heightmap(std::string filename);
  virtual ~Heightmap() = default;

private:
  std::vector<glm::vec3> m_vertices;
  std::vector<uint32_t> m_indices;
};
#endif // HEIGHTMAP_H_
