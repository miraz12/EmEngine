#ifndef GLTFOBJECT_H_
#define GLTFOBJECT_H_

#include <Objects/GraphicsObject.hpp>
#include <queue>
#include <string>
#include <tiny_gltf.h>

class GltfObject : public GraphicsObject {
public:
  GltfObject(std::string filename);
  virtual ~GltfObject() = default;

private:
  void loadModel(tinygltf::Model &model);
  void loadNode(tinygltf::Model &model, tinygltf::Node &node, glm::mat4 mat);
  void loadMaterials(tinygltf::Model &model);
  void loadTextures(tinygltf::Model &model);
  void loadMeshes(tinygltf::Model &model);
  void generateCollisionShape();

  btTriangleMesh *m_mesh;
  std::vector<std::string> m_texIds;
};

#endif // GLTFOBJECT_H_
