#include "glm/ext/matrix_transform.hpp"
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btConvexShape.h>
#include <BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <Rendering/Primitive.hpp>
#include <cstdint>
#include <memory>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "GltfObject.hpp"
#include <Rendering/Material.hpp>
#include <Rendering/Mesh.hpp>
#include <Rendering/Node.hpp>
#include <cstddef>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ostream>

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
  generateCollisionShape();
}

void GltfObject::loadModel(tinygltf::Model &model) {
  loadTextures(model);
  loadMaterials(model);
  loadMeshes(model);

  const tinygltf::Scene &scene = model.scenes[model.defaultScene];
  for (auto &n : scene.nodes) {
    loadNode(model, model.nodes[n], glm::identity<glm::mat4>());
  }
}

void GltfObject::loadNode(tinygltf::Model &model, tinygltf::Node &node, glm::mat4 mat) {
  glm::mat4 modelMat = mat;
  if (!node.matrix.empty()) {
    modelMat *= glm::mat4(node.matrix[0]);
  }
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
  if (node.mesh >= 0) {
    Node *n = new Node;
    n->mesh = node.mesh;
    n->nodeMat = modelMat;
    p_nodes.push_back(n);
  }
  for (int32_t c : node.children) {
    loadNode(model, model.nodes[c], modelMat);
  }
}

void GltfObject::loadMaterials(tinygltf::Model &model) {
  p_numMats = model.materials.size();
  p_materials = std::make_unique<Material[]>(p_numMats);
  uint32_t numNodes = 0;
  for (auto &mat : model.materials) {
    uint32_t materialMast = 0;
    if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) {
      materialMast = materialMast | (1 << 0);
      p_materials[numNodes].m_baseColorTexture =
          m_texIds.at(mat.pbrMetallicRoughness.baseColorTexture.index);
    }
    if (mat.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
      materialMast = materialMast | (1 << 1);
      p_materials[numNodes].m_metallicRoughnessTexture =
          m_texIds.at(mat.pbrMetallicRoughness.metallicRoughnessTexture.index);
    }
    if (mat.emissiveTexture.index >= 0) {
      materialMast = materialMast | (1 << 2);
      p_materials[numNodes].m_emissiveTexture = m_texIds.at(mat.emissiveTexture.index);
    }
    if (mat.occlusionTexture.index >= 0) {
      materialMast = materialMast | (1 << 3);
      p_materials[numNodes].m_occlusionTexture = m_texIds.at(mat.occlusionTexture.index);
    }
    if (mat.normalTexture.index >= 0) {
      materialMast = materialMast | (1 << 4);
      p_materials[numNodes].m_normalTexture = m_texIds.at(mat.normalTexture.index);
    }

    p_materials[numNodes].m_material = materialMast;
    p_materials[numNodes].m_baseColorFactor = glm::vec3(
        mat.pbrMetallicRoughness.baseColorFactor[0], mat.pbrMetallicRoughness.baseColorFactor[1],
        mat.pbrMetallicRoughness.baseColorFactor[2]);
    p_materials[numNodes].m_roughnessFactor = mat.pbrMetallicRoughness.roughnessFactor;
    p_materials[numNodes].m_metallicFactor = mat.pbrMetallicRoughness.metallicFactor;
    p_materials[numNodes].m_emissiveFactor =
        glm::vec3(mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2]);
    p_materials[numNodes].m_doubleSided = mat.doubleSided;
    p_materials[numNodes].m_alphaMode = mat.alphaMode;
    p_materials[numNodes].m_alphaCutoff = mat.alphaCutoff;
    numNodes++;
  }
}

void GltfObject::loadTextures(tinygltf::Model &model) {

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
      uint32_t id =
          texMan.loadTexture(GL_RGBA, format, type, image.width, image.height, &image.image.at(0));
      m_texIds.push_back(std::to_string(id));
    }
  }
}

void GltfObject::loadMeshes(tinygltf::Model &model) {
  p_numMeshes = model.meshes.size();
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  uint32_t meshCount = 0;
  for (auto &mesh : model.meshes) {
    p_meshes[meshCount].m_primitives = std::make_unique<Primitive[]>(mesh.primitives.size());
    for (auto &primitive : mesh.primitives) {
      uint32_t vao;
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      Primitive *newPrim = &p_meshes[meshCount].m_primitives[p_meshes[meshCount].numPrims++];
      m_mesh = new btTriangleMesh();
      newPrim->m_vao = vao;
      newPrim->m_mode = primitive.mode;
      newPrim->m_material = primitive.material;

      // Check if using element buffer
      if (primitive.indices != -1) {
        tinygltf::Accessor accessor = model.accessors[primitive.indices];
        newPrim->m_count = accessor.count;
        newPrim->m_type = accessor.componentType;
        newPrim->m_offset = accessor.byteOffset;
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
        newPrim->m_ebo = ebo;
        newPrim->m_drawType = 1;
        const void *indicesData = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
          const uint16_t *indices = reinterpret_cast<const uint16_t *>(indicesData);
          for (uint32_t i = 0; i < newPrim->m_count; i += 3) {
            int32_t index1 = indices[i];
            int32_t index2 = indices[i + 1];
            int32_t index3 = indices[i + 2];
            m_mesh->addTriangleIndices(index1, index2, index3);
          }
        } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
          const uint32_t *indices = reinterpret_cast<const uint32_t *>(indicesData);
          for (uint32_t i = 0; i < newPrim->m_count; i += 3) {
            int32_t index1 = indices[i];
            int32_t index2 = indices[i + 1];
            int32_t index3 = indices[i + 2];
            m_mesh->addTriangleIndices(index1, index2, index3);
          }
        }
      }
      // Load all vertex attributes
      for (auto &attrib : primitive.attributes) {
        tinygltf::Accessor accessor = model.accessors[attrib.second];
        const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        uint32_t loc = 0;
        if (attrib.first == "POSITION") {
          loc = 0;
          const float *positions = reinterpret_cast<const float *>(
              &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
          int numVertices = accessor.count;
          for (int i = 0; i < numVertices; i += 3) {
            // clang-format off
              btVector3 vertex0(positions[i * 3],
                                positions[i * 3 + 1],
                                positions[i * 3 + 2]);
              btVector3 vertex1(positions[(i + 1) * 3],
                                positions[(i + 1) * 3 + 1],
                                positions[(i + 1) * 3 + 2]);
              btVector3 vertex2(positions[(i + 2) * 3],
                                positions[(i + 2) * 3 + 1],
                                positions[(i + 2) * 3 + 2]);
            // clang-format on
            m_mesh->addTriangle(vertex0, vertex1, vertex2);
          }
        } else if (attrib.first == "NORMAL") {
          loc = 1;
        } else if (attrib.first == "TANGENT") {
          loc = 2;
        } else if (attrib.first == "TEXCOORD_0") {
          loc = 3;
        }

        uint32_t vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(bufferView.target, bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);

        int32_t byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        glVertexAttribPointer(loc, accessor.type, accessor.componentType, accessor.normalized,
                              byteStride, (void *)(sizeof(char) * (accessor.byteOffset)));
        glEnableVertexAttribArray(loc);

        Primitive::AttribInfo attribInfo;
        attribInfo.vbo = loc;
        attribInfo.type = accessor.type;
        attribInfo.componentType = accessor.componentType;
        attribInfo.byteOffset = accessor.byteOffset;
        attribInfo.byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        attribInfo.normalized = accessor.normalized;
        newPrim->attributes[attrib.first] = attribInfo;
        loc++;
      }
    }
    meshCount++;
  }
}

void GltfObject::generateCollisionShape() {
  btConvexShape *cShape = new btConvexTriangleMeshShape(m_mesh);
  btShapeHull *cHull = new btShapeHull(cShape);
  cHull->buildHull(cShape->getMargin());
  btConvexHullShape *chShape = new btConvexHullShape();
  cHull->numTriangles();
  for (int i = 0; i < cHull->numTriangles(); ++i) {
    chShape->addPoint(cHull->getVertexPointer()[cHull->getIndexPointer()[i]]);
  }
  chShape->optimizeConvexHull();
  p_coll = chShape;
}
