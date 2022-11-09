#include "Mesh.hpp"
#include <iostream>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

Mesh::Mesh(std::string filename) {

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;

  std::string err;
  std::string warn;
  bool success = loader.LoadBinaryFromFile(&model, &err, &warn, filename);

  if (!warn.empty()) {
    std::cout << "Warn: %s\n" << warn << std::endl;
  }

  if (!err.empty()) {
    std::cout << "ERR: %s\n" << err << std::endl;
  }
  if (!success) {
    std::cout << "Failed to load .glTF " << filename << std::endl;
  }

  tinygltf::Scene scene =
      model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];

  for (size_t i = 0; i < scene.nodes.size(); i++) {
    std::cout << "node.name : " << scene.nodes[i] << std::endl;
  }

  for (size_t i = 0; i < model.bufferViews.size(); i++) {
    const tinygltf::BufferView &bufferView = model.bufferViews[i];
    if (bufferView.target == 0) {
      std::cout << "WARN: bufferView.target is zero" << std::endl;
      continue; // Unsupported bufferView.
    }

    int sparse_accessor = -1;
    for (size_t a_i = 0; a_i < model.accessors.size(); ++a_i) {
      const auto &accessor = model.accessors[a_i];
      if (accessor.bufferView == i) {
        std::cout << i << " is used by accessor " << a_i << std::endl;
        if (accessor.sparse.isSparse) {
          std::cout
              << "WARN: this bufferView has at least one sparse accessor to "
                 "it. We are going to load the data as patched by this "
                 "sparse accessor, not the original data"
              << std::endl;
          sparse_accessor = a_i;
          break;
        }
      }
    }
  }
}

void Mesh::draw(ShaderProgram& prog) {}
