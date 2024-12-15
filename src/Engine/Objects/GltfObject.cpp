#include "Rendering/Skin.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#undef int
#include <tiny_gltf.h>

#include "GltfObject.hpp"
#include <Rendering/Material.hpp>
#include <Rendering/Mesh.hpp>
#include <Rendering/Node.hpp>
#include <Rendering/Primitive.hpp>

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

static std::string
GetFilePathExtension(const std::string& FileName)
{
  // TODO: Use std::filesystem
  if (FileName.find_last_of(".") != std::string::npos)
    return FileName.substr(FileName.find_last_of(".") + 1);
  return "";
}

GltfObject::GltfObject(std::string filename)
  : m_filename(filename)
{
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

void
GltfObject::loadModel(tinygltf::Model& model)
{
  loadTextures(model);
  loadMaterials(model);
  loadMeshes(model);
  loadAnimation(model);
  loadSkins(model);

  p_numNodes = model.nodes.size();
  p_nodes = std::make_unique<Node[]>(p_numNodes);

  for (u32 i = 0; i < model.nodes.size(); i++) {
    loadNode(model.nodes[i], i);
  }
}

void
GltfObject::loadNode(tinygltf::Node& node, u32 nodeIdx)
{

  p_nodes[nodeIdx].mesh = node.mesh;
  p_nodes[nodeIdx].skin = node.skin;
  p_nodes[nodeIdx].name = node.name;
  p_nodes[nodeIdx].nodeMat = glm::mat4(1.0f);

  p_nodes[nodeIdx].nodeMat =
    glm::mat4(1.0f); // Identity matrix for local transformation

  if (!node.matrix.empty()) {
    p_nodes[nodeIdx].nodeMat = glm::make_mat4x4(node.matrix.data());
  } else {
    if (!node.translation.empty()) {
      p_nodes[nodeIdx].trans = glm::vec3(
        node.translation[0], node.translation[1], node.translation[2]);
    }
    if (!node.rotation.empty()) {
      glm::quat rotationQuat(
        node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
      p_nodes[nodeIdx].rot = rotationQuat;
    }
    if (!node.scale.empty()) {
      p_nodes[nodeIdx].scale =
        glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
    }
  }
  for (i32 c : node.children) {
    p_nodes[c].parent = nodeIdx;
  }
}

void
GltfObject::loadMaterials(tinygltf::Model& model)
{
  p_numMats = model.materials.size();
  p_materials = std::make_unique<Material[]>(p_numMats);
  u32 numNodes = 0;
  for (auto& mat : model.materials) {
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

void
GltfObject::loadTextures(tinygltf::Model& model)
{

  for (auto& tex : model.textures) {
    TextureManager& texMan = TextureManager::getInstance();

    tinygltf::Image& image = model.images[tex.source];
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
      u32 id = texMan.loadTexture(
        GL_RGBA, format, type, image.width, image.height, &image.image.at(0));
      m_texIds.push_back(std::to_string(id));
    }
  }
}

void
GltfObject::loadMeshes(tinygltf::Model& model)
{
  p_numMeshes = model.meshes.size();
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  u32 meshCount = 0;
  for (auto& mesh : model.meshes) {
    p_meshes[meshCount].m_primitives =
      std::make_unique<Primitive[]>(mesh.primitives.size());
    for (auto& primitive : mesh.primitives) {
      u32 vao;
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      Primitive* newPrim =
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
        const tinygltf::BufferView& bufferView =
          model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
        GLuint ebo;
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset,
                     GL_STATIC_DRAW);
        newPrim->m_ebo = ebo;
        newPrim->m_drawType = 1;
        const void* indicesData =
          &buffer.data[bufferView.byteOffset + accessor.byteOffset];
        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
          const u16* indices = reinterpret_cast<const u16*>(indicesData);
          for (u32 i = 0; i < newPrim->m_count; i += 3) {
            i32 index1 = indices[i];
            i32 index2 = indices[i + 1];
            i32 index3 = indices[i + 2];
            m_mesh->addTriangleIndices(index1, index2, index3);
          }
        } else if (accessor.componentType ==
                   TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
          const u32* indices = reinterpret_cast<const u32*>(indicesData);
          for (u32 i = 0; i < newPrim->m_count; i += 3) {
            i32 index1 = indices[i];
            i32 index2 = indices[i + 1];
            i32 index3 = indices[i + 2];
            m_mesh->addTriangleIndices(index1, index2, index3);
          }
        }
      }
      // Load all vertex attributes
      for (const auto& attrib : primitive.attributes) {
        tinygltf::Accessor accessor = model.accessors[attrib.second];
        const tinygltf::BufferView& bufferView =
          model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

        u32 loc = 0;
        if (attrib.first == "POSITION") {
          loc = 0;
          auto positions = reinterpret_cast<const float*>(
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
        } else if (attrib.first == "JOINTS_0") {
          loc = 4;
        } else if (attrib.first == "WEIGHTS_0") {
          loc = 5;
        }

        u32 vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(bufferView.target,
                     bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset,
                     GL_STATIC_DRAW);

        i32 byteStride =
          accessor.ByteStride(model.bufferViews[accessor.bufferView]);
        glVertexAttribPointer(loc,
                              accessor.type,
                              accessor.componentType,
                              accessor.normalized,
                              byteStride,
                              (void*)(sizeof(char) * (accessor.byteOffset)));
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

void
GltfObject::loadAnimation(tinygltf::Model& model)
{
  std::cout << "Num animations: " << model.animations.size() << std::endl;
  p_numAnimations = model.animations.size();
  p_animations = std::make_unique<Animation[]>(p_numAnimations);
  u32 animCount = 0;
  for (tinygltf::Animation& anim : model.animations) {
    Animation animation;
    animation.name = anim.name;
    if (anim.name.empty()) {
      animation.name = std::to_string(p_numAnimations);
    }

    // Samplers
    for (auto& samp : anim.samplers) {
      AnimationSampler sampler{};

      if (samp.interpolation == "LINEAR") {
        sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
      } else if (samp.interpolation == "STEP") {
        sampler.interpolation = AnimationSampler::InterpolationType::STEP;
      } else if (samp.interpolation == "CUBICSPLINE") {
        sampler.interpolation =
          AnimationSampler::InterpolationType::CUBICSPLINE;
      }

      // Read sampler input time values
      {
        const tinygltf::Accessor& accessor = model.accessors[samp.input];
        const tinygltf::BufferView& bufferView =
          model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        const void* dataPtr =
          &buffer.data[accessor.byteOffset + bufferView.byteOffset];
        const float* buf = static_cast<const float*>(dataPtr);
        for (size_t index = 0; index < accessor.count; index++) {
          sampler.inputs.push_back(buf[index]);
        }

        for (auto input : sampler.inputs) {
          if (input < animation.start) {
            animation.start = input;
          };
          if (input > animation.end) {
            animation.end = input;
          }
        }
      }

      // Read sampler output T/R/S values
      {
        const tinygltf::Accessor& accessor = model.accessors[samp.output];
        const tinygltf::BufferView& bufferView =
          model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

        const void* dataPtr =
          &buffer.data[accessor.byteOffset + bufferView.byteOffset];

        switch (accessor.type) {
          case TINYGLTF_TYPE_VEC3: {
            const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
            for (size_t index = 0; index < accessor.count; index++) {
              sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
              sampler.outputs.push_back(buf[index][0]);
              sampler.outputs.push_back(buf[index][1]);
              sampler.outputs.push_back(buf[index][2]);
            }
            break;
          }
          case TINYGLTF_TYPE_VEC4: {
            const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
            for (size_t index = 0; index < accessor.count; index++) {
              sampler.outputsVec4.push_back(buf[index]);
              sampler.outputs.push_back(buf[index][0]);
              sampler.outputs.push_back(buf[index][1]);
              sampler.outputs.push_back(buf[index][2]);
              sampler.outputs.push_back(buf[index][3]);
            }
            break;
          }
          default: {
            std::cout << "unknown type" << std::endl;
            break;
          }
        }
      }

      animation.samplers.push_back(sampler);
    }

    // Channels
    for (auto& source : anim.channels) {
      AnimationChannel channel{};

      if (source.target_path == "rotation") {
        channel.path = AnimationChannel::PathType::ROTATION;
      }
      if (source.target_path == "translation") {
        channel.path = AnimationChannel::PathType::TRANSLATION;
      }
      if (source.target_path == "scale") {
        channel.path = AnimationChannel::PathType::SCALE;
      }
      if (source.target_path == "weights") {
        std::cout << "weights not yet supported, skipping channel" << std::endl;
        continue;
      }
      channel.samplerIndex = source.sampler;
      channel.node = source.target_node;
      if (channel.node < 0) {
        continue;
      }

      animation.channels.push_back(channel);
    }

    p_animations[animCount++] = animation;
  }
}

void
GltfObject::loadSkins(tinygltf::Model& model)
{
  p_numSkins = model.skins.size();
  p_skins = std::make_unique<Skin[]>(p_numSkins);

  u32 numSkins = 0;
  for (tinygltf::Skin& source : model.skins) {
    p_skins[numSkins].name = source.name;

    // Find skeleton root node
    if (source.skeleton > -1) {
      p_skins[numSkins].skeletonRoot = source.skeleton;
    }

    // Find joint nodes
    for (i32 jointIndex : source.joints) {
      if (jointIndex >= 0) {
        p_skins[numSkins].joints.push_back(jointIndex);
      }
    }

    // Get inverse bind matrices from buffer
    if (source.inverseBindMatrices != -1) {
      const tinygltf::Accessor& accessor =
        model.accessors[source.inverseBindMatrices];
      const tinygltf::BufferView& bufferView =
        model.bufferViews[accessor.bufferView];
      const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

      auto matrices = reinterpret_cast<const float*>(
        &buffer.data[bufferView.byteOffset + accessor.byteOffset]);

      for (size_t i = 0; i < accessor.count; ++i) {
        glm::mat4 invMatrix =
          glm::make_mat4x4(matrices + (i * 16)); // 4x4 matrix = 16 floats
        p_skins[numSkins].inverseBindMatrices.push_back(invMatrix);
      }
    }
    numSkins++;
  }
}

void
GltfObject::generateCollisionShape()
{
  // Generate convex shape from a triangle mesh
  btConvexShape* cShape = new btConvexTriangleMeshShape(
    m_mesh); // m_mesh is assumed to be a btTriangleMesh
  btShapeHull* cHull = new btShapeHull(cShape);

  // Build hull from the triangle mesh shape with margin
  cHull->buildHull(cShape->getMargin());

  // Create a new convex hull shape to hold the simplified vertices
  btConvexHullShape* chShape = new btConvexHullShape();

  // Check if the hull has valid data
  if (cHull->numVertices() > 0) {

    // Iterate through all vertices in the hull and add them to the convex hull
    // shape
    for (i32 i = 0; i < cHull->numVertices(); ++i) {
      // Add the vertex to the convex hull shape
      chShape->addPoint(cHull->getVertexPointer()[i]);
    }

    // Optional: Optimize the convex hull shape for better performance
    chShape->optimizeConvexHull();
  }

  // Assign the final collision shape
  p_coll = chShape;
}
