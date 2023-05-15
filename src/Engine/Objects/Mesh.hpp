#ifndef MESH_H_
#define MESH_H_

#include <ShaderPrograms/ShaderProgram.hpp>
#include <map>
#include <vector>

#include "GraphicsObject.hpp"
#include "tiny_gltf.h"
class Mesh : public GraphicsObject {
public:
  Mesh();
  virtual ~Mesh();
  void draw(const ShaderProgram &sPrg) override;
  void drawGeom(const ShaderProgram &sPrg) override;

  void LoadFlile(std::string filename);

private:
  void drawModelNodes(tinygltf::Model &model, tinygltf::Node &node, const ShaderProgram &sPrg);
  void drawMesh(tinygltf::Model &model, tinygltf::Mesh &mesh, const ShaderProgram &sPrg);
  void drawMeshGeom(tinygltf::Model &model, tinygltf::Mesh &mesh, const ShaderProgram &sPrg);
  void drawModelGeomNodes(tinygltf::Model &model, tinygltf::Node &node, const ShaderProgram &sPrg);

  void loadModel(tinygltf::Model &model);
  void loadMaterials();
  void loadTextures(tinygltf::Model &model);
  void loadNode(tinygltf::Model &model, tinygltf::Node &node);

  std::map<int, unsigned int> m_buffers;
  tinygltf::Model m_model;
  unsigned int m_vao;
};
#endif // MESH_H_
