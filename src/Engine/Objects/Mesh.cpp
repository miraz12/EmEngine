#include <Objects/GraphicsObject.hpp>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <iostream>

#include "Mesh.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#include "Managers/TextureManager.hpp"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static std::string GetFilePathExtension(const std::string &FileName) {
  if (FileName.find_last_of(".") != std::string::npos)
    return FileName.substr(FileName.find_last_of(".") + 1);
  return "";
}

Mesh::Mesh(ShaderProgram &shaderProgram) : GraphicsObject(shaderProgram) {}

Mesh::~Mesh() { glDeleteVertexArrays(1, &m_vaoAndEbos.first); }

void Mesh::draw(Camera &cam, glm::mat4 model) {
  glBindVertexArray(m_vaoAndEbos.first);
  p_shaderProgram.use();
  cam.bindProjViewMatrix(p_shaderProgram.getUniformLocation("projMatrix"),
                         p_shaderProgram.getUniformLocation("viewMatrix"));
  glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1,
                     GL_FALSE, glm::value_ptr(model));
  GLint tex[5] = {0, 1, 2, 3, 4};
  glUniform1iv(p_shaderProgram.getUniformLocation("textures"), 5, tex);

  const tinygltf::Scene &scene = m_model.scenes[m_model.defaultScene];
  for (size_t i = 0; i < scene.nodes.size(); ++i) {
    drawModelNodes(m_vaoAndEbos, m_model, m_model.nodes[scene.nodes[i]]);
  }

  glBindVertexArray(0);
}

void Mesh::drawModelNodes(
    const std::pair<unsigned int, std::map<int, unsigned int>> &vaoAndEbos,
    tinygltf::Model &model, tinygltf::Node &node) {
  if ((node.mesh >= 0) &&
      (static_cast<unsigned int>(node.mesh) < model.meshes.size())) {
    drawMesh(vaoAndEbos.second, model, model.meshes[node.mesh]);
  }
  for (size_t i = 0; i < node.children.size(); i++) {
    drawModelNodes(vaoAndEbos, model, model.nodes[node.children[i]]);
  }
}

void Mesh::drawMesh(const std::map<int, unsigned int> &vbos,
                    tinygltf::Model &model, tinygltf::Mesh &mesh) {
  TextureManager &texMan = TextureManager::getInstance();
  for (size_t i = 0; i < mesh.primitives.size(); ++i) {
    tinygltf::Primitive primitive = mesh.primitives[i];

    tinygltf::Material m = model.materials[primitive.material];
    int texIdx = m.pbrMetallicRoughness.baseColorTexture.index;
    if (texIdx >= 0) {
      glActiveTexture(GL_TEXTURE0 + 0);
      texMan.bindTexture(texIdx);
    }
    texIdx = m.pbrMetallicRoughness.metallicRoughnessTexture.index;
    if (texIdx >= 0) {
      glActiveTexture(GL_TEXTURE0 + 1);
      texMan.bindTexture(texIdx);
    }

    texIdx = m.emissiveTexture.index;
    if (texIdx >= 0) {
      glActiveTexture(GL_TEXTURE0 + 2);
      texMan.bindTexture(texIdx);
    }

    texIdx = m.occlusionTexture.index;
    if (texIdx >= 0) {
      glActiveTexture(GL_TEXTURE0 + 3);
      texMan.bindTexture(texIdx);
    }

    texIdx = m.normalTexture.index;
    if (texIdx >= 0) {
      glActiveTexture(GL_TEXTURE0 + 4);
      texMan.bindTexture(texIdx);
    }

    // if (m.doubleSided) {
    //   glDisable(GL_CULL_FACE);
    // } else {
    //   glEnable(GL_CULL_FACE);
    // }
    // if (m.alphaMode == "BLEND") {
    //   glEnable(GL_BLEND);
    //   glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
    //                       GL_ONE_MINUS_SRC_ALPHA);
    //   glBlendEquation(GL_FUNC_ADD);
    // } else {
    //   glDisable(GL_BLEND);
    // }
    if (primitive.indices != -1) {
      tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));
      glDrawElements(primitive.mode, indexAccessor.count,
                     indexAccessor.componentType,
                     (void *)(sizeof(char) * (indexAccessor.byteOffset)));
    } else {
      for (auto &attrib : primitive.attributes) {
        tinygltf::Accessor accessor = model.accessors[attrib.second];
        unsigned int loc = vbos.at(accessor.bufferView);
        glBindBuffer(GL_ARRAY_BUFFER, loc);
        int byteStride =
            accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        glVertexAttribPointer(loc, accessor.type, accessor.componentType,
                              accessor.normalized, byteStride,
                              (void *)(sizeof(char) * (accessor.byteOffset)));
        glEnableVertexAttribArray(loc);
      }
      tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
      glBindBuffer(GL_ARRAY_BUFFER, vbos.at(indexAccessor.bufferView));
      glDrawArrays(primitive.mode, 0, model.accessors[primitive.indices].count);
    }
  }
}

void Mesh::LoadFlile(std::string filename) {
  std::string ext = GetFilePathExtension(filename);
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;
  bool ret = false;
  std::cout << "Loading: " << filename << std::endl;
  if (ext.compare("glb") == 0) {
    // assume binary glTF.
    ret = loader.LoadBinaryFromFile(&m_model, &err, &warn, filename.c_str());
  } else {
    // assume ascii glTF.
    ret = loader.LoadASCIIFromFile(&m_model, &err, &warn, filename.c_str());
  }

  if (!warn.empty()) {
    printf("Warn: %s\n", warn.c_str());
  }

  if (!err.empty()) {
    printf("ERR: %s\n", err.c_str());
  }
  if (!ret) {
    printf("Failed to load .glTF : %s\n", filename.c_str());
    exit(-1);
  }

  m_vaoAndEbos = loadModel(m_model);
}

void Mesh::bindModelNodes(std::map<int, unsigned int> &vbos,
                          tinygltf::Model &model, tinygltf::Node &node) {
  if ((node.mesh >= 0) &&
      (static_cast<unsigned int>(node.mesh) < model.meshes.size())) {
    bindMesh(vbos, model, model.meshes[node.mesh]);
  }

  // Load child nodes
  for (size_t i = 0; i < node.children.size(); i++) {
    assert((node.children[i] >= 0) &&
           (static_cast<unsigned int>(node.children[i]) < model.nodes.size()));
    bindModelNodes(vbos, model, model.nodes[node.children[i]]);
  }
}

std::pair<unsigned int, std::map<int, unsigned int>>
Mesh::loadModel(tinygltf::Model &model) {
  std::map<int, unsigned int> vbos;
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  loadTextures(model);

  const tinygltf::Scene &scene = model.scenes[model.defaultScene];
  for (size_t i = 0; i < scene.nodes.size(); ++i) {
    assert((scene.nodes[i] >= 0) &&
           (static_cast<unsigned int>(scene.nodes[i]) < model.nodes.size()));
    // bindModelNodes(vbos, model, model.nodes[scene.nodes[i]]);
    loadNode(model, model.nodes[scene.nodes[i]], vbos);
  }

  glBindVertexArray(0);
  // cleanup vbos but do not delete index buffers yet
  // for (auto it = vbos.cbegin(); it != vbos.cend();) {
  //   tinygltf::BufferView bufferView = model.bufferViews[it->first];
  //   if (bufferView.target != GL_ELEMENT_ARRAY_BUFFER) {
  //     glDeleteBuffers(1, &vbos[it->first]);
  //     vbos.erase(it++);
  //   } else {
  //     ++it;
  //   }
  // }

  return {vao, vbos};
}

void Mesh::bindMesh(std::map<int, unsigned int> &vbos, tinygltf::Model &model,
                    tinygltf::Mesh &mesh) {
  for (size_t i = 0; i < model.bufferViews.size(); ++i) {
    const tinygltf::BufferView &bufferView = model.bufferViews[i];
    const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
    // std::cout << "bufferview.target " << bufferView.target << std::endl;

    GLuint vbo;
    glGenBuffers(1, &vbo);
    vbos[i] = vbo;
    glBindBuffer(bufferView.target, vbo);

    // std::cout << "buffer.data.size = " << buffer.data.size()
    //           << ", bufferview.byteOffset = " << bufferView.byteOffset
    //           << std::endl;

    glBufferData(bufferView.target, bufferView.byteLength,
                 &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
  }

  for (size_t i = 0; i < mesh.primitives.size(); ++i) {
    tinygltf::Primitive primitive = mesh.primitives[i];
    tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];

    for (auto &attrib : primitive.attributes) {
      tinygltf::Accessor accessor = model.accessors[attrib.second];
      glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

      int size = 1;
      if (accessor.type != TINYGLTF_TYPE_SCALAR) {
        size = accessor.type;
      }

      int vaa = -1;
      if (attrib.first.compare("POSITION") == 0)
        vaa = 0;
      if (attrib.first.compare("NORMAL") == 0)
        vaa = 1;
      if (attrib.first.compare("TANGENT") == 0)
        vaa = 2;
      if (attrib.first.compare("TEXCOORD_0") == 0)
        vaa = 3;
      if (vaa > -1) {
        int byteStride =
            accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        assert(byteStride != -1);
        glVertexAttribPointer(
            vaa, size, accessor.componentType,
            accessor.normalized ? GL_TRUE : GL_FALSE, byteStride,
            (void *)(sizeof(char) * (indexAccessor.byteOffset)));
        glEnableVertexAttribArray(vaa);
      } else
        std::cout << "vaa missing: " << attrib.first << std::endl;
    }
  }
}
void Mesh::loadTextures(tinygltf::Model &model) {
  for (auto &tex : model.textures) {
    TextureManager &texMan = TextureManager::getInstance();

    tinygltf::Image &image = model.images[tex.source];

    if (tex.source > -1) {
      GLenum format = GL_RGBA;
      if (image.component == 1) {
        format = GL_RED;
      } else if (image.component == 2) {
        format = GL_RG;
      } else if (image.component == 3) {
        format = GL_RGB;
      } else if (image.component == 4) {
        format = GL_RGBA;
      } else {
        std::cout << "WARNING: no matching format." << std::endl;
      }

      GLenum type = GL_UNSIGNED_BYTE;
      if (image.bits == 8) {
        type = GL_UNSIGNED_BYTE;
      } else if (image.bits == 16) {
        type = GL_UNSIGNED_SHORT;
      } else {
        std::cout << "WARNING: no matching type." << std::endl;
      }

      texMan.loadTexture(format, type, image.width, image.height,
                         &image.image.at(0));
    }
  }
}

void Mesh::loadNode(tinygltf::Model &model, tinygltf::Node &node,
                    std::map<int, unsigned int> &vbos) {
  if (node.mesh > -1) {
    const tinygltf::Mesh mesh = model.meshes.at(node.mesh);

    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
      const tinygltf::BufferView &bufferView = model.bufferViews[i];
      const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

      GLuint vbo;
      glGenBuffers(1, &vbo);
      vbos[i] = vbo;
      glBindBuffer(bufferView.target, vbo);

      glBufferData(bufferView.target, bufferView.byteLength,
                   &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
    }

    for (auto &primitive : mesh.primitives) {
      // Get buffer data for vertex normals
      if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
        const tinygltf::Accessor &accessor =
            model.accessors[primitive.attributes.find("POSITION")->second];
        const tinygltf::BufferView &view =
            model.bufferViews[accessor.bufferView];
        glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);
        glVertexAttribPointer(
            0, accessor.type, accessor.componentType,
            accessor.normalized ? GL_TRUE : GL_FALSE,
            accessor.ByteStride(model.bufferViews[accessor.bufferView]),
            reinterpret_cast<const float *>(
                &(model.buffers[view.buffer]
                      .data[accessor.byteOffset + view.byteOffset])));
        glEnableVertexAttribArray(0);
      }
      // Get buffer data for vertex normals
      if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
        const tinygltf::Accessor &accessor =
            model.accessors[primitive.attributes.find("NORMAL")->second];
        const tinygltf::BufferView &view =
            model.bufferViews[accessor.bufferView];

        glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);
        glVertexAttribPointer(
            1, accessor.type, accessor.componentType,
            accessor.normalized ? GL_TRUE : GL_FALSE,
            accessor.ByteStride(model.bufferViews[accessor.bufferView]),
            reinterpret_cast<const float *>(
                &(model.buffers[view.buffer]
                      .data[accessor.byteOffset + view.byteOffset])));
        glEnableVertexAttribArray(1);
      }
      // Get buffer data for vertex texture coordinates
      // glTF supports multiple sets, we only load the first one
      if (primitive.attributes.find("TEXCOORD_0") !=
          primitive.attributes.end()) {
        const tinygltf::Accessor &accessor =
            model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
        const tinygltf::BufferView &view =
            model.bufferViews[accessor.bufferView];

        glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);
        glVertexAttribPointer(
            2, accessor.type, accessor.componentType,
            accessor.normalized ? GL_TRUE : GL_FALSE,
            accessor.ByteStride(model.bufferViews[accessor.bufferView]),
            reinterpret_cast<const float *>(
                &(model.buffers[view.buffer]
                      .data[accessor.byteOffset + view.byteOffset])));
        glEnableVertexAttribArray(2);
      }
      // POI: This sample uses normal mapping, so we also need to load the
      // tangents from the glTF file
      if (primitive.attributes.find("TANGENT") != primitive.attributes.end()) {
        const tinygltf::Accessor &accessor =
            model.accessors[primitive.attributes.find("TANGENT")->second];
        const tinygltf::BufferView &view =
            model.bufferViews[accessor.bufferView];

        glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);
        glVertexAttribPointer(
            3, accessor.type, accessor.componentType,
            accessor.normalized ? GL_TRUE : GL_FALSE,
            accessor.ByteStride(model.bufferViews[accessor.bufferView]),
            reinterpret_cast<const float *>(
                &(model.buffers[view.buffer]
                      .data[accessor.byteOffset + view.byteOffset])));
        glEnableVertexAttribArray(3);
      }
    }
  }
}
