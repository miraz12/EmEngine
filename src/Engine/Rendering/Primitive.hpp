#ifndef PRIMITIVE_H_
#define PRIMITIVE_H_

class Primitive {
public:
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

  int32_t m_material{-1};

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

#endif // PRIMITIVE_H_
