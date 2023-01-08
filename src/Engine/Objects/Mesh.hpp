#ifndef MESH_H_
#define MESH_H_

#include <map>
#include <vector>

#include "tiny_gltf.h"

#include "GraphicsObject.hpp"
class Mesh : public GraphicsObject {
public:
  Mesh(ShaderProgram &p);
  virtual ~Mesh();
  void draw(Camera &cam) override;

  void LoadFlile(std::string filename);

private:
  void drawModelNodes(
      const std::pair<unsigned int, std::map<int, unsigned int>> &vaoAndEbos,
      tinygltf::Model &model, tinygltf::Node &node);
  void drawMesh(const std::map<int, unsigned int> &vbos, tinygltf::Model &model,
                tinygltf::Mesh &mesh);
  void bindModelNodes(std::map<int, unsigned int> &vbos, tinygltf::Model &model,
                      tinygltf::Node &node);
  std::pair<unsigned int, std::map<int, unsigned int>>
  bindModel(tinygltf::Model &model);
  void bindMesh(std::map<int, unsigned int> &vbos, tinygltf::Model &model,
                tinygltf::Mesh &mesh);

  struct GLBufferState {
    unsigned int vb;
  };

  struct GLMeshState {
    std::vector<unsigned int> diffuseTex; // for each primitive in mesh
  };

  struct GLProgramState {
    std::map<std::string, int> attribs;
    std::map<std::string, int> uniforms;
  };

  struct GLCurvesState {
    unsigned int vb; // vertex buffer
    size_t count;    // byte count
  };

  std::map<int, GLBufferState> gBufferState;
  std::map<std::string, GLMeshState> gMeshState;
  std::map<int, GLCurvesState> gCurvesMesh;
  GLProgramState gGLProgramState;
  std::pair<unsigned int, std::map<int, unsigned int>> m_vaoAndEbos;
  tinygltf::Model m_model;
  unsigned int texid;
  glm::mat4 m_modelMatrix;

};
#endif // MESH_H_
