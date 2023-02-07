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
  void drawModelNodes(const std::pair<unsigned int, std::map<int, unsigned int>> &vaoAndEbos,
                      tinygltf::Model &model, tinygltf::Node &node);
  void drawMesh(const std::map<int, unsigned int> &vbos, tinygltf::Model &model,
                tinygltf::Mesh &mesh);
  std::pair<unsigned int, std::map<int, unsigned int>> loadModel(tinygltf::Model &model);
  void loadMaterials();
  void loadTextures(tinygltf::Model &model);
  void loadNode(tinygltf::Model &model, tinygltf::Node &node, std::map<int, unsigned int> &vbos);

  std::pair<unsigned int, std::map<int, unsigned int>> m_vaoAndEbos;
  tinygltf::Model m_model;
};
#endif // MESH_H_
