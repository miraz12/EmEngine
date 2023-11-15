#ifndef GLTFOBJECT_H_
#define GLTFOBJECT_H_

#include "GraphicsObject.hpp"

class GltfObject : public GraphicsObject {
public:
  GltfObject(std::string filename);
  virtual ~GltfObject() = default;

  std::string_view getFileName() { return m_filename; };

private:
  void loadModel(tinygltf::Model &model);
  void loadNode(tinygltf::Model &model, tinygltf::Node &node, glm::mat4 mat);
  void loadMaterials(tinygltf::Model &model);
  void loadTextures(tinygltf::Model &model);
  void loadMeshes(tinygltf::Model &model);
  void loadAnimation(tinygltf::Model &model);
  void loadSkins(tinygltf::Model &model);
  void loadJoints(tinygltf::Model &model);
  void generateCollisionShape();

  btTriangleMesh *m_mesh;
  std::vector<std::string> m_texIds;
  std::string m_filename;
};

#endif // GLTFOBJECT_H_
