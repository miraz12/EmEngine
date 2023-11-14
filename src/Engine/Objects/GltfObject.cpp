#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GltfObject.hpp"
#include <Rendering/Material.hpp>
#include <Rendering/Mesh.hpp>
#include <Rendering/Node.hpp>
#include <Rendering/Primitive.hpp>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static std::string GetFilePathExtension(const std::string &FileName) {
  if (FileName.find_last_of(".") != std::string::npos)
    return FileName.substr(FileName.find_last_of(".") + 1);
  return "";
}

GltfObject::GltfObject(std::string filename) : m_filename(filename) {
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
  loadAnimation(model);

  const tinygltf::Scene &scene = model.scenes[model.defaultScene];
  for (auto &n : scene.nodes) {
    loadNode(model, model.nodes[n], glm::identity<glm::mat4>());
  }
}

void GltfObject::loadNode(tinygltf::Model &model, tinygltf::Node &node,
                          glm::mat4 mat) {
  glm::mat4 modelMat = mat;
  if (!node.matrix.empty()) {
    modelMat *= glm::mat4(node.matrix[0]);
  }
  if (!node.rotation.empty()) {
    modelMat = glm::rotate(modelMat, (float)(node.rotation[0]),
                           glm::vec3(1.0, 0.0, 0.0));
    modelMat = glm::rotate(modelMat, (float)(node.rotation[1]),
                           glm::vec3(0.0, 1.0, 0.0));
    modelMat = glm::rotate(modelMat, (float)(node.rotation[2]),
                           glm::vec3(0.0, 0.0, 1.0));
  }
  if (!node.scale.empty()) {
    modelMat = glm::scale(
        modelMat, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
  }
  if (!node.translation.empty()) {
    modelMat = glm::translate(modelMat, glm::vec3(node.translation[0],
                                                  node.translation[1],
                                                  node.translation[2]));
  }
  if (node.mesh >= 0) {
    Node *n = new Node;
    n->mesh = node.mesh;
    n->nodeMat = modelMat;
    p_nodes.push_back(n);
  }
  for (i32 c : node.children) {
    loadNode(model, model.nodes[c], modelMat);
  }
}

void GltfObject::loadMaterials(tinygltf::Model &model) {
  p_numMats = model.materials.size();
  p_materials = std::make_unique<Material[]>(p_numMats);
  u32 numNodes = 0;
  for (auto &mat : model.materials) {
    u32 materialMast = 0;
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
      p_materials[numNodes].m_emissiveTexture =
          m_texIds.at(mat.emissiveTexture.index);
    }
    if (mat.occlusionTexture.index >= 0) {
      materialMast = materialMast | (1 << 3);
      p_materials[numNodes].m_occlusionTexture =
          m_texIds.at(mat.occlusionTexture.index);
    }
    if (mat.normalTexture.index >= 0) {
      materialMast = materialMast | (1 << 4);
      p_materials[numNodes].m_normalTexture =
          m_texIds.at(mat.normalTexture.index);
    }

    p_materials[numNodes].m_material = materialMast;
    p_materials[numNodes].m_baseColorFactor =
        glm::vec3(mat.pbrMetallicRoughness.baseColorFactor[0],
                  mat.pbrMetallicRoughness.baseColorFactor[1],
                  mat.pbrMetallicRoughness.baseColorFactor[2]);
    p_materials[numNodes].m_roughnessFactor =
        mat.pbrMetallicRoughness.roughnessFactor;
    p_materials[numNodes].m_metallicFactor =
        mat.pbrMetallicRoughness.metallicFactor;
    p_materials[numNodes].m_emissiveFactor = glm::vec3(
        mat.emissiveFactor[0], mat.emissiveFactor[1], mat.emissiveFactor[2]);
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
      u32 id = texMan.loadTexture(GL_RGBA, format, type, image.width,
                                  image.height, &image.image.at(0));
      m_texIds.push_back(std::to_string(id));
    }
  }
}

void GltfObject::loadMeshes(tinygltf::Model &model) {
  p_numMeshes = model.meshes.size();
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  u32 meshCount = 0;
  for (auto &mesh : model.meshes) {
    p_meshes[meshCount].m_primitives =
        std::make_unique<Primitive[]>(mesh.primitives.size());
    for (auto &primitive : mesh.primitives) {
      u32 vao;
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      Primitive *newPrim =
          &p_meshes[meshCount].m_primitives[p_meshes[meshCount].numPrims++];
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
        const tinygltf::BufferView &bufferView =
            model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset,
                     GL_STATIC_DRAW);
        newPrim->m_ebo = ebo;
        newPrim->m_drawType = 1;
        const void *indicesData =
            &buffer.data[bufferView.byteOffset + accessor.byteOffset];
        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
          const u16 *indices = reinterpret_cast<const u16 *>(indicesData);
          for (u32 i = 0; i < newPrim->m_count; i += 3) {
            i32 index1 = indices[i];
            i32 index2 = indices[i + 1];
            i32 index3 = indices[i + 2];
            m_mesh->addTriangleIndices(index1, index2, index3);
          }
        } else if (accessor.componentType ==
                   TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
          const u32 *indices = reinterpret_cast<const u32 *>(indicesData);
          for (u32 i = 0; i < newPrim->m_count; i += 3) {
            i32 index1 = indices[i];
            i32 index2 = indices[i + 1];
            i32 index3 = indices[i + 2];
            m_mesh->addTriangleIndices(index1, index2, index3);
          }
        }
      }
      // Load all vertex attributes
      for (auto &attrib : primitive.attributes) {
        tinygltf::Accessor accessor = model.accessors[attrib.second];
        const tinygltf::BufferView &bufferView =
            model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        u32 loc = 0;
        if (attrib.first == "POSITION") {
          loc = 0;
          const float *positions = reinterpret_cast<const float *>(
              &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
          u32 numVertices = accessor.count;
          for (u32 i = 0; i < numVertices; i += 3) {
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

        u32 vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(bufferView.target, bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset,
                     GL_STATIC_DRAW);

        i32 byteStride =
            accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        glVertexAttribPointer(loc, accessor.type, accessor.componentType,
                              accessor.normalized, byteStride,
                              (void *)(sizeof(char) * (accessor.byteOffset)));
        glEnableVertexAttribArray(loc);

        Primitive::AttribInfo attribInfo;
        attribInfo.vbo = loc;
        attribInfo.type = accessor.type;
        attribInfo.componentType = accessor.componentType;
        attribInfo.byteOffset = accessor.byteOffset;
        attribInfo.byteStride =
            accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        attribInfo.normalized = accessor.normalized;
        newPrim->attributes[attrib.first] = attribInfo;
        loc++;
      }
    }
    meshCount++;
  }
}

void GltfObject::loadAnimation(tinygltf::Model &model) {
  p_numAnimations = model.animations.size();
  std::cout << "Num animations: " << p_numAnimations << std::endl;
  p_animations = std::make_unique<Animation[]>(p_numAnimations);

  u32 numNodes = 0;
  for (tinygltf::Animation &anim : model.animations) {
    Animation animation{};
    p_animations[numNodes].name = anim.name;
    if (anim.name.empty()) {
      p_animations[numNodes].name = std::to_string(p_numAnimations);
    }

    // Samplers
    for (auto &samp : anim.samplers) {
      Animation::AnimationSampler sampler{};

      if (samp.interpolation == "LINEAR") {
        sampler.interpolation =
            Animation::AnimationSampler::InterpolationType::LINEAR;
      }
      if (samp.interpolation == "STEP") {
        sampler.interpolation =
            Animation::AnimationSampler::InterpolationType::STEP;
      }
      if (samp.interpolation == "CUBICSPLINE") {
        sampler.interpolation =
            Animation::AnimationSampler::InterpolationType::CUBICSPLINE;
      }

      // Read sampler input time values
      {
        const tinygltf::Accessor &accessor = model.accessors[samp.input];
        const tinygltf::BufferView &bufferView =
            model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        const void *dataPtr =
            &buffer.data[accessor.byteOffset + bufferView.byteOffset];
        const float *buf = static_cast<const float *>(dataPtr);
        for (size_t index = 0; index < accessor.count; index++) {
          sampler.inputs.push_back(buf[index]);
        }

        for (auto input : sampler.inputs) {
          if (input < p_animations[numNodes].start) {
            p_animations[numNodes].start = input;
          };
          if (input > p_animations[numNodes].end) {
            p_animations[numNodes].end = input;
          }
        }
      }

      // Read sampler output T/R/S values
      {
        const tinygltf::Accessor &accessor = model.accessors[samp.output];
        const tinygltf::BufferView &bufferView =
            model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        const void *dataPtr =
            &buffer.data[accessor.byteOffset + bufferView.byteOffset];

        switch (accessor.type) {
        case TINYGLTF_TYPE_VEC3: {
          const glm::vec3 *buf = static_cast<const glm::vec3 *>(dataPtr);
          for (size_t index = 0; index < accessor.count; index++) {
            sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
          }
          break;
        }
        case TINYGLTF_TYPE_VEC4: {
          const glm::vec4 *buf = static_cast<const glm::vec4 *>(dataPtr);
          for (size_t index = 0; index < accessor.count; index++) {
            sampler.outputsVec4.push_back(buf[index]);
          }
          break;
        }
        default: {
          std::cout << "unknown type" << std::endl;
          break;
        }
        }
      }

      p_animations[numNodes].samplers.push_back(sampler);
    }

    // Channels
    for (auto &source : anim.channels) {
      Animation::AnimationChannel channel{};

      if (source.target_path == "rotation") {
        channel.path = Animation::AnimationChannel::PathType::ROTATION;
      }
      if (source.target_path == "translation") {
        channel.path = Animation::AnimationChannel::PathType::TRANSLATION;
      }
      if (source.target_path == "scale") {
        channel.path = Animation::AnimationChannel::PathType::SCALE;
      }
      if (source.target_path == "weights") {
        std::cout << "weights not yet supported, skipping channel" << std::endl;
        continue;
      }
      channel.samplerIndex = source.sampler;
      // channel.node = nodeFromIndex(source.target_node);
      // if (!channel.node) {
      // continue;
      // }

      p_animations[numNodes].channels.push_back(channel);
    }

    numNodes++;
  }
}

// void GltfObject::loadSkins(tinygltf::Model &model) {
//   for (tinygltf::Skin &source : model.skins) {
//     Skin *newSkin = new Skin{};
//     newSkin->name = source.name;

//     // Find skeleton root node
//     if (source.skeleton > -1) {
//       newSkin->skeletonRoot = nodeFromIndex(source.skeleton);
//     }

//     // Find joint nodes
//     for (u32 jointIndex : source.joints) {
//       Node *node = nodeFromIndex(jointIndex);
//       if (node) {
//         newSkin->joints.push_back(nodeFromIndex(jointIndex));
//       }
//     }

//     // Get inverse bind matrices from buffer
//     if (source.inverseBindMatrices > -1) {
//       const tinygltf::Accessor &accessor =
//           model.accessors[source.inverseBindMatrices];
//       const tinygltf::BufferView &bufferView =
//           model.bufferViews[accessor.bufferView];
//       const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];
//       newSkin->inverseBindMatrices.resize(accessor.count);
//       memcpy(newSkin->inverseBindMatrices.data(),
//              &buffer.data[accessor.byteOffset + bufferView.byteOffset],
//              accessor.count * sizeof(glm::mat4));
//     }

//     skins.push_back(newSkin);
//   }
// }
void GltfObject::loadJoints(tinygltf::Model & /* model */) {}

void GltfObject::generateCollisionShape() {
  btConvexShape *cShape = new btConvexTriangleMeshShape(m_mesh);
  btShapeHull *cHull = new btShapeHull(cShape);
  cHull->buildHull(cShape->getMargin());
  btConvexHullShape *chShape = new btConvexHullShape();
  cHull->numTriangles();
  for (u32 i = 0; i < cHull->numTriangles(); ++i) {
    chShape->addPoint(cHull->getVertexPointer()[cHull->getIndexPointer()[i]]);
  }
  chShape->optimizeConvexHull();
  p_coll = chShape;
}
