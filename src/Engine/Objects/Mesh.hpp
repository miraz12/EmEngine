#ifndef MESH_H_
#define MESH_H_

#include <map>
#include <vector>

#include "GraphicsObject.hpp"
#include "tiny_gltf.h"
class Mesh : public GraphicsObject {
public:
  Mesh(ShaderProgram &p);
  virtual ~Mesh();
  void draw(Camera &cam, glm::mat4 model) override;

  void LoadFlile(std::string filename);

private:
  void drawModelNodes(tinygltf::Model &model, tinygltf::Node &node);
  void drawMesh(tinygltf::Model &model, tinygltf::Mesh &mesh);
  void loadModel(tinygltf::Model &model);
  void loadMaterials();
  void loadTextures(tinygltf::Model &model);
  void loadNode(tinygltf::Model &model, tinygltf::Node &node);

  std::map<int, unsigned int> m_buffers;
  tinygltf::Model m_model;
  unsigned int m_vao;
};
#endif // MESH_H_
