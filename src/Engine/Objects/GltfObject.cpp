#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "GltfObject.hpp"
#include <Rendering/Material.hpp>
#include <Rendering/MeshObj.hpp>
#include <cstddef>
#include <iostream>
#include <ostream>

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static std::string GetFilePathExtension(const std::string &FileName) {
  if (FileName.find_last_of(".") != std::string::npos)
    return FileName.substr(FileName.find_last_of(".") + 1);
  return "";
}

GltfObject::GltfObject(std::string filename) {
  std::string ext = GetFilePathExtension(filename);
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;
  bool ret = false;
  tinygltf::Model model;
  std::cout << "Loading: " << filename << std::endl;
  if (ext.compare("glb") == 0) {
    // assume binary glTF.
    ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename.c_str());
  } else {
    // assume ascii glTF.
    ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename.c_str());
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

  loadModel(model);
  std::cout << "Load done!" << std::endl;
}
void GltfObject::draw(const ShaderProgram &sPrg) {
  glUniformMatrix4fv(sPrg.getUniformLocation("modelMatrix"), 1, GL_FALSE,
                     glm::value_ptr(p_modelMatrix));

  for (size_t i = 0; i < p_meshes.size(); i++) {
    p_materials.at(p_meshes[i]->m_material)->bind(sPrg);
    p_meshes.at(i)->draw(sPrg);
  }
}
void GltfObject::drawGeom(const ShaderProgram &sPrg) {
  glUniformMatrix4fv(sPrg.getUniformLocation("modelMatrix"), 1, GL_FALSE,
                     glm::value_ptr(p_modelMatrix));

  for (size_t i = 0; i < p_meshes.size(); i++) {
    p_meshes[i]->draw(sPrg);
  }
}

void GltfObject::loadModel(tinygltf::Model &model) {
  loadTextures(model);
  loadMaterials(model);
  loadMeshes(model);
}

void GltfObject::loadMaterials(tinygltf::Model &model) {
  for (auto &mat : model.materials) {
    Material *newmat = new Material;
    unsigned int materialMast = 0;
    if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) {
      materialMast = materialMast | (1 << 0);
      newmat->m_baseColorTexture = std::to_string(mat.pbrMetallicRoughness.baseColorTexture.index);
    }
    if (mat.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
      materialMast = materialMast | (1 << 1);
      newmat->m_metallicRoughnessTexture =
          std::to_string(mat.pbrMetallicRoughness.metallicRoughnessTexture.index);
    }
    if (mat.emissiveTexture.index >= 0) {
      materialMast = materialMast | (1 << 2);
      newmat->m_emissiveTexture = std::to_string(mat.emissiveTexture.index);
    }
    if (mat.occlusionTexture.index >= 0) {
      materialMast = materialMast | (1 << 3);
      newmat->m_occlusionTexture = std::to_string(mat.occlusionTexture.index);
    }
    if (mat.normalTexture.index >= 0) {
      // materialMast = materialMast | (1 << 4);
      // newmat->m_normalTexture = std::to_string(mat.normalTexture.index);
    }

    newmat->m_material = materialMast;
    newmat->m_baseColorFactor = glm::vec3(1.0f);
    newmat->m_emissiveFactor = glm::vec3(1.0f);
    newmat->m_roughnessFactor = 1.0f;
    newmat->m_metallicFactor = 1.0f;
    newmat->m_doubleSided = mat.doubleSided;
    newmat->m_alphaMode = mat.alphaMode;
    p_materials.push_back(newmat);
  }
}

void GltfObject::loadTextures(tinygltf::Model &model) {

  int i = 0;
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

      texMan.loadTexture(std::to_string(i++), GL_RGBA, format, type, image.width, image.height,
                         &image.image.at(0));
    }
  }
}

void GltfObject::loadMeshes(tinygltf::Model &model) {
  for (auto &mesh : model.meshes) {
    for (auto &primitive : mesh.primitives) {
      unsigned int vao;
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      MeshObj *newmesh = new MeshObj;
      newmesh->m_vao = vao;
      newmesh->m_mode = primitive.mode;
      newmesh->m_material = primitive.material;

      // Check if using element buffer
      if (primitive.indices != -1) {
        GLuint ebo;
        glGenBuffers(1, &ebo);
        tinygltf::Accessor accessor = model.accessors[primitive.indices];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
        newmesh->m_ebo = ebo;
        newmesh->m_eboCount = accessor.count;
        newmesh->m_eboType = accessor.componentType;
        newmesh->m_eboOffset = accessor.byteOffset;
      }
      // Load all vertex attributes
      for (auto &attrib : primitive.attributes) {
        unsigned int loc = 0;
        std::cout << attrib.first << std::endl;
        if (attrib.first == "POSITION") {
          loc = 0;
        } else if (attrib.first == "NORMAL") {
          loc = 1;
        } else if (attrib.first == "TANGENT") {
          loc = 2;
        } else if (attrib.first == "TEXCOORD_0") {
          loc = 3;
        }
        tinygltf::Accessor accessor = model.accessors[attrib.second];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(bufferView.target, bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

        int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        glVertexAttribPointer(loc, accessor.type, accessor.componentType, accessor.normalized,
                              byteStride, (void *)(sizeof(char) * (accessor.byteOffset)));
        glEnableVertexAttribArray(loc);

        MeshObj::AttribInfo attribInfo;
        attribInfo.vbo = loc;
        attribInfo.type = accessor.type;
        attribInfo.componentType = accessor.componentType;
        attribInfo.byteOffset = accessor.byteOffset;
        attribInfo.byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        attribInfo.normalized = accessor.normalized;
        newmesh->attributes[attrib.first] = attribInfo;
        loc++;
      }
      p_meshes.push_back(newmesh);
    }
  }
  glBindVertexArray(0);
}
