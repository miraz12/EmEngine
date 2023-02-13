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

Mesh::~Mesh() { glDeleteVertexArrays(1, &m_vao); }

void Mesh::draw(Camera &cam, glm::mat4 /* model */) {
  p_shaderProgram.use();
  cam.bindProjViewMatrix(p_shaderProgram.getUniformLocation("projMatrix"),
                         p_shaderProgram.getUniformLocation("viewMatrix"));
  // glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
  // glm::value_ptr(model));
  GLint tex[5] = {0, 1, 2, 3, 4};
  glUniform1iv(p_shaderProgram.getUniformLocation("textures"), 5, tex);
  glBindVertexArray(m_vao);

  const tinygltf::Scene &scene = m_model.scenes[m_model.defaultScene];
  for (size_t i = 0; i < scene.nodes.size(); ++i) {
    drawModelNodes(m_model, m_model.nodes[scene.nodes[i]]);
  }
}

void Mesh::drawModelNodes(tinygltf::Model &model, tinygltf::Node &node) {
  glm::mat4 modelMat = glm::identity<glm::mat4>();
  if (!node.rotation.empty()) {
    modelMat = glm::rotate(modelMat, (float)(node.rotation[0]), glm::vec3(1.0, 0.0, 0.0));
    modelMat = glm::rotate(modelMat, (float)(node.rotation[1]), glm::vec3(0.0, 1.0, 0.0));
    modelMat = glm::rotate(modelMat, (float)(node.rotation[2]), glm::vec3(0.0, 0.0, 1.0));
  }
  if (!node.scale.empty()) {
    modelMat = glm::scale(modelMat, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
  }
  if (!node.translation.empty()) {
    modelMat = glm::translate(
        modelMat, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
  }
  glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
                     glm::value_ptr(modelMat));

  if ((node.mesh >= 0) && (static_cast<unsigned int>(node.mesh) < model.meshes.size())) {
    drawMesh(model, model.meshes[node.mesh]);
  }
  for (size_t i = 0; i < node.children.size(); i++) {
    drawModelNodes(model, model.nodes[node.children[i]]);
  }
}
void Mesh::drawMesh(tinygltf::Model &model, tinygltf::Mesh &mesh) {
  TextureManager &texMan = TextureManager::getInstance();
  for (size_t i = 0; i < mesh.primitives.size(); ++i) {
    tinygltf::Primitive primitive = mesh.primitives[i];

    // TODO : Add flags for texutres to use in shader.

    // Packed material.
    // 0 = Base color
    // 1 = Metall/Roughness
    // 2 = Emissive
    // 3 = Occlusion
    // 4 = Normal
    unsigned int material = 0;
    tinygltf::Material m = model.materials[primitive.material];
    int texIdx = m.pbrMetallicRoughness.baseColorTexture.index;
    if (texIdx >= 0) {
      glUniform3f(p_shaderProgram.getUniformLocation("baseColorFactor"),
                  m.pbrMetallicRoughness.baseColorFactor[0],
                  m.pbrMetallicRoughness.baseColorFactor[1],
                  m.pbrMetallicRoughness.baseColorFactor[2]);
      glActiveTexture(GL_TEXTURE0 + 0);
      texMan.bindTexture(texIdx);
      material = material | (1 << 0);
    }
    texIdx = m.pbrMetallicRoughness.metallicRoughnessTexture.index;
    if (texIdx >= 0) {
      glUniform1f(p_shaderProgram.getUniformLocation("roughnessFactor"),
                  m.pbrMetallicRoughness.roughnessFactor);
      glUniform1f(p_shaderProgram.getUniformLocation("metallicFactor"),
                  m.pbrMetallicRoughness.metallicFactor);
      glActiveTexture(GL_TEXTURE0 + 1);
      texMan.bindTexture(texIdx);
      material = material | (1 << 1);
    }

    texIdx = m.emissiveTexture.index;
    if (texIdx >= 0) {
      glUniform3f(p_shaderProgram.getUniformLocation("emissiveFactor"), m.emissiveFactor[0],
                  m.emissiveFactor[1], m.emissiveFactor[2]);
      glActiveTexture(GL_TEXTURE0 + 2);
      texMan.bindTexture(texIdx);
      material = material | (1 << 2);
    }

    texIdx = m.occlusionTexture.index;
    if (texIdx >= 0) {
      glActiveTexture(GL_TEXTURE0 + 3);
      texMan.bindTexture(texIdx);
      material = material | (1 << 3);
    }

    texIdx = m.normalTexture.index;
    if (texIdx >= 0) {
      glActiveTexture(GL_TEXTURE0 + 4);
      texMan.bindTexture(texIdx);
      material = material | (1 << 4);
    }

    glUniform1i(p_shaderProgram.getUniformLocation("material"), material);

    if (m.doubleSided) {
      glDisable(GL_CULL_FACE);
    } else {
      glEnable(GL_CULL_FACE);
    }
    if (m.alphaMode == "BLEND") {
      glEnable(GL_BLEND);
      glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      glBlendEquation(GL_FUNC_ADD);
    } else {
      glDisable(GL_BLEND);
    }
    for (auto &attrib : primitive.attributes) {
      tinygltf::Accessor accessor = model.accessors[attrib.second];
      int loc = p_shaderProgram.getAttribLocation(attrib.first);
      if (loc > -1 && !m_buffers.empty()) {
        unsigned int vbo = m_buffers.at(accessor.bufferView);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        glVertexAttribPointer(loc, accessor.type, accessor.componentType, accessor.normalized,
                              byteStride, (void *)(sizeof(char) * (accessor.byteOffset)));
        glEnableVertexAttribArray(loc);
      }
    }

    if (primitive.indices != -1) {
      tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers.at(indexAccessor.bufferView));
      glDrawElements(primitive.mode, indexAccessor.count, indexAccessor.componentType,
                     (void *)(sizeof(char) * (indexAccessor.byteOffset)));
    } else {
      for (auto &attrib : primitive.attributes) {
        tinygltf::Accessor accessor = model.accessors[attrib.second];
        unsigned int loc = m_buffers.at(accessor.bufferView);
        glBindBuffer(GL_ARRAY_BUFFER, loc);
        int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        glVertexAttribPointer(loc, accessor.type, accessor.componentType, accessor.normalized,
                              byteStride, (void *)(sizeof(char) * (accessor.byteOffset)));
        glEnableVertexAttribArray(loc);
      }
      tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
      glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(indexAccessor.bufferView));
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

  loadModel(m_model);
  std::cout << "Load done!" << std::endl;
}

void Mesh::loadModel(tinygltf::Model &model) {
  glGenVertexArrays(1, &m_vao);
  glBindVertexArray(m_vao);
  loadTextures(model);

  const tinygltf::Scene &scene = model.scenes[model.defaultScene];
  for (size_t i = 0; i < scene.nodes.size(); ++i) {
    assert((scene.nodes[i] >= 0) &&
           (static_cast<unsigned int>(scene.nodes[i]) < model.nodes.size()));
    loadNode(model, model.nodes[scene.nodes[i]]);
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

      texMan.loadTexture(format, type, image.width, image.height, &image.image.at(0));
    }
  }
}

void Mesh::loadNode(tinygltf::Model &model, tinygltf::Node &node) {
  if (node.mesh > -1) {
    const tinygltf::Mesh mesh = model.meshes.at(node.mesh);

    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
      const tinygltf::BufferView &bufferView = model.bufferViews[i];
      const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
      if (bufferView.target != 0) {
        GLuint vbo;
        glGenBuffers(1, &vbo);
        m_buffers[i] = vbo;
        std::cout << "buffer[" << i << "] is :" << vbo << std::endl;
        glBindBuffer(bufferView.target, vbo);
        glBufferData(bufferView.target, bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
      } else {
        std::cout << "Warning buffer view target = 0!" << std::endl;
      }
    }

    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
      tinygltf::Primitive primitive = mesh.primitives[i];
      for (auto &attrib : primitive.attributes) {
        tinygltf::Accessor accessor = model.accessors[attrib.second];
        int loc = p_shaderProgram.getAttribLocation(attrib.first);
        if (loc > -1) {
          unsigned int vbo = m_buffers.at(accessor.bufferView);
          glBindBuffer(GL_ARRAY_BUFFER, vbo);
          int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
          glVertexAttribPointer(loc, accessor.type, accessor.componentType, accessor.normalized,
                                byteStride, (void *)(sizeof(char) * (accessor.byteOffset)));
          glEnableVertexAttribArray(loc);
        }
      }
    }
  }
}
