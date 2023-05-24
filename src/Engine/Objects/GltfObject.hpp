#ifndef GLTFOBJECT_H_
#define GLTFOBJECT_H_

#include <Objects/GraphicsObject.hpp>
#include <string>
#include <tiny_gltf.h>

class GltfObject : public GraphicsObject {
public:
  GltfObject(std::string filename);
  virtual ~GltfObject() = default;
  void draw(const ShaderProgram &sPrg) override;
  void drawGeom(const ShaderProgram &sPrg) override;

private:
  void loadModel(tinygltf::Model &model);
  void loadNode(tinygltf::Model &model, tinygltf::Node &node, glm::mat4 mat);
  void loadMaterials(tinygltf::Model &model);
  void loadTextures(tinygltf::Model &model);
  void loadMeshes(tinygltf::Model &model);
};

#endif // GLTFOBJECT_H_
