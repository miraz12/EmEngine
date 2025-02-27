#ifndef HEIGHTMAP_H_
#define HEIGHTMAP_H_

#include "GraphicsObject.hpp"

class Heightmap final : public GraphicsObject
{
public:
  Heightmap() = delete;
  explicit Heightmap(std::string filename);
  ~Heightmap() override = default;

  std::string_view getFileName() { return m_filename; };

private:
  std::vector<glm::vec3> m_vertices;
  std::vector<float> m_data;
  std::vector<u32> m_indices;
  std::string m_filename;
};
#endif // HEIGHTMAP_H_
