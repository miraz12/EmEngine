#ifndef MESHOBJ_H_
#define MESHOBJ_H_

#include <ShaderPrograms/ShaderProgram.hpp>
#include <cstddef>
#include <map>
#include <vector>

class Mesh {
public:
  Mesh() = default;
  virtual ~Mesh() = default;
  void draw();

  struct AttribInfo {
    unsigned int vbo;
    size_t byteOffset;
    size_t byteStride;
    bool normalized;
    int componentType;
    size_t count;
    int type;
  };

  int m_material{0};

  unsigned int m_vbo;

  unsigned int m_drawType{0}; // Either 0 = drawArrays or 1 

  unsigned int m_ebo;
  size_t m_count;
  unsigned int m_type;
  size_t m_offset;

  unsigned int m_vao;
  unsigned int m_mode;
  // std::vector<unsigned char> data;
  std::map<std::string, AttribInfo> attributes;
};

#endif // MESHOBJ_H_
