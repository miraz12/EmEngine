#ifndef PRIMITIVE_H_
#define PRIMITIVE_H_

class Primitive {
public:
  void draw();

  struct AttribInfo {
    u32 vbo;
    size_t byteOffset;
    size_t byteStride;
    bool normalized;
    i32 componentType;
    size_t count;
    i32 type;
  };

  i32 m_material{-1};

  u32 m_vbo;

  u32 m_drawType{0}; // Either 0 = drawArrays or 1

  u32 m_ebo;
  u32 m_count;
  u32 m_type;
  u32 m_offset;

  u32 m_vao;
  u32 m_mode;
  std::map<std::string, AttribInfo> attributes;
};

#endif // PRIMITIVE_H_
