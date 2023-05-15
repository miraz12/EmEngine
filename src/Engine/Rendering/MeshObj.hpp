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
  void draw(const ShaderProgram &sPrg);

  struct AttribInfo {
    unsigned int vbo;
    size_t byteOffset;
    size_t byteStride;
    bool normalized;
    int componentType;
    size_t count;
    int type;
  };


  unsigned int m_material;

  unsigned int m_vbo;

  unsigned int m_ebo;
  unsigned int m_eboCount;
  unsigned int m_eboType;
  unsigned int m_eboOffset;

  unsigned int m_vao;
  unsigned int m_mode;
  // std::vector<unsigned char> data;
  std::map<std::string, AttribInfo> attributes;
  glm::mat4 m_model;
};

#endif // MESHOBJ_H_
