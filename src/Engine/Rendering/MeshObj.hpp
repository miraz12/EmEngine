#ifndef MESHOBJ_H_
#define MESHOBJ_H_

#include <ShaderPrograms/ShaderProgram.hpp>
#include <cstddef>
#include <glm/glm.hpp>
#include <map>
#include <vector>

class MeshObj {
public:
  MeshObj() = default;
  virtual ~MeshObj() = default;
  void draw();

  struct AttribInfo {
    uint32_t vbo;
    size_t byteOffset;
    size_t byteStride;
    bool normalized;
    int32_t componentType;
    size_t count;
    int32_t type;
  };

  int32_t m_material{0};

  uint32_t m_vbo;

  uint32_t m_drawType{0}; // Either 0 = drawArrays or 1

  uint32_t m_ebo;
  uint32_t m_count;
  uint32_t m_type;
  uint32_t m_offset;

  uint32_t m_vao;
  uint32_t m_mode;
  // std::vector<unsigned char> data;
  std::map<std::string, AttribInfo> attributes;
};

#endif // MESHOBJ_H_
