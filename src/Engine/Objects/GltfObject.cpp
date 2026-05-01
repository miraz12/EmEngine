#include "Rendering/Skin.hpp"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#undef int
#include <tiny_gltf.h>

#include "GltfObject.hpp"
#include <Graphics/RenderResources.hpp>
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
  auto& resources = gfx::RenderResources::getInstance();

  for (size_t texIdx = 0; texIdx < model.textures.size(); texIdx++) {
    auto& tex = model.textures[texIdx];
    tinygltf::Image& image = model.images[tex.source];

    if (tex.source > -1) {
      // Map component count to PixelFormat
      gfx::PixelFormat format = gfx::PixelFormat::RGBA8;
      if (image.component == 1) {
        format = gfx::PixelFormat::R8;
      } else if (image.component == 2) {
        format = gfx::PixelFormat::RG8;
      } else if (image.component == 3) {
        format = gfx::PixelFormat::RGB8;
      } else if (image.component == 4) {
        format = gfx::PixelFormat::RGBA8;
      } else {
        std::cout << "WARNING: no matching format." << std::endl;
      }

      // Validate bit depth (currently only 8-bit supported in PixelFormat
      // enums)
      if (image.bits != 8 && image.bits != 16) {
        std::cout << "WARNING: unsupported bit depth: " << image.bits
                  << std::endl;
      }

      // Generate unique texture name based on model filename and texture index
      std::string texName = m_filename + "_tex" + std::to_string(texIdx);

      gfx::TextureCreateInfo texInfo{};
      texInfo.width = static_cast<u32>(image.width);
      texInfo.height = static_cast<u32>(image.height);
      texInfo.format = format;
      texInfo.mipLevels = 1;
      texInfo.initialData = image.image.data();
      texInfo.debugName = texName.c_str();

      resources.createTexture2D(texName, texInfo);
      m_texIds.push_back(texName);
    }
  }
}

// Interleaved vertex layout for glTF meshes
// Attribute locations match shader expectations:
//   0 = POSITION (vec3), 1 = NORMAL (vec3), 2 = TANGENT (vec4),
//   3 = TEXCOORD_0 (vec2), 4 = JOINTS_0 (uvec4), 5 = WEIGHTS_0 (vec4)
struct InterleavedVertex
{
  glm::vec3 position{ 0.0f };
  glm::vec3 normal{ 0.0f, 1.0f, 0.0f };
  glm::vec4 tangent{ 1.0f, 0.0f, 0.0f, 1.0f };
  glm::vec2 texcoord{ 0.0f };
  glm::u16vec4 joints{ 0 };
  glm::vec4 weights{ 1.0f, 0.0f, 0.0f, 0.0f };
};

// Helper to get attribute data pointer from glTF accessor
static const void*
getAccessorDataPtr(const tinygltf::Model& model,
                   const tinygltf::Accessor& accessor)
{
  const auto& bufferView = model.bufferViews[accessor.bufferView];
  const auto& buffer = model.buffers[bufferView.buffer];
  return &buffer.data[bufferView.byteOffset + accessor.byteOffset];
}

void
GltfObject::loadMeshes(tinygltf::Model& model)
{
  auto& resources = gfx::RenderResources::getInstance();

  p_numMeshes = model.meshes.size();
  p_meshes = std::make_unique<Mesh[]>(p_numMeshes);
  u32 meshCount = 0;

  for (auto& mesh : model.meshes) {
    p_meshes[meshCount].m_primitives =
      std::make_unique<Primitive[]>(mesh.primitives.size());

    for (auto& primitive : mesh.primitives) {
      Primitive* newPrim =
        &p_meshes[meshCount].m_primitives[p_meshes[meshCount].numPrims++];
      m_mesh = new btTriangleMesh();
      newPrim->m_material = primitive.material;

      // Determine vertex count from POSITION attribute (required)
      u32 vertexCount = 0;
      auto posIt = primitive.attributes.find("POSITION");
      if (posIt != primitive.attributes.end()) {
        vertexCount = model.accessors[posIt->second].count;
      }

      // Create interleaved vertex buffer
      std::vector<InterleavedVertex> vertices(vertexCount);

      // Fill vertex data from each attribute
      for (const auto& attrib : primitive.attributes) {
        const auto& accessor = model.accessors[attrib.second];
        const void* dataPtr = getAccessorDataPtr(model, accessor);

        if (attrib.first == "POSITION") {
          const auto* positions = static_cast<const float*>(dataPtr);
          for (u32 idx = 0; idx < vertexCount; idx++) {
            vertices[idx].position = glm::vec3(positions[idx * 3 + 0],
                                               positions[idx * 3 + 1],
                                               positions[idx * 3 + 2]);
          }
          // Build collision mesh from positions
          for (u32 idx = 0; idx + 2 < vertexCount; idx += 3) {
            btVector3 v0(positions[idx * 3],
                         positions[idx * 3 + 1],
                         positions[idx * 3 + 2]);
            btVector3 v1(positions[(idx + 1) * 3],
                         positions[(idx + 1) * 3 + 1],
                         positions[(idx + 1) * 3 + 2]);
            btVector3 v2(positions[(idx + 2) * 3],
                         positions[(idx + 2) * 3 + 1],
                         positions[(idx + 2) * 3 + 2]);
            m_mesh->addTriangle(v0, v1, v2);
          }
        } else if (attrib.first == "NORMAL") {
          const auto* normals = static_cast<const float*>(dataPtr);
          for (u32 idx = 0; idx < vertexCount; idx++) {
            vertices[idx].normal = glm::vec3(
              normals[idx * 3 + 0], normals[idx * 3 + 1], normals[idx * 3 + 2]);
          }
        } else if (attrib.first == "TANGENT") {
          const auto* tangents = static_cast<const float*>(dataPtr);
          for (u32 idx = 0; idx < vertexCount; idx++) {
            vertices[idx].tangent = glm::vec4(tangents[idx * 4 + 0],
                                              tangents[idx * 4 + 1],
                                              tangents[idx * 4 + 2],
                                              tangents[idx * 4 + 3]);
          }
        } else if (attrib.first == "TEXCOORD_0") {
          const auto* texcoords = static_cast<const float*>(dataPtr);
          for (u32 idx = 0; idx < vertexCount; idx++) {
            vertices[idx].texcoord =
              glm::vec2(texcoords[idx * 2 + 0], texcoords[idx * 2 + 1]);
          }
        } else if (attrib.first == "JOINTS_0") {
          // Joints can be u8 or u16
          if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
            const auto* joints = static_cast<const u8*>(dataPtr);
            for (u32 idx = 0; idx < vertexCount; idx++) {
              vertices[idx].joints = glm::u16vec4(joints[idx * 4 + 0],
                                                  joints[idx * 4 + 1],
                                                  joints[idx * 4 + 2],
                                                  joints[idx * 4 + 3]);
            }
          } else {
            const auto* joints = static_cast<const u16*>(dataPtr);
            for (u32 idx = 0; idx < vertexCount; idx++) {
              vertices[idx].joints = glm::u16vec4(joints[idx * 4 + 0],
                                                  joints[idx * 4 + 1],
                                                  joints[idx * 4 + 2],
                                                  joints[idx * 4 + 3]);
            }
          }
        } else if (attrib.first == "WEIGHTS_0") {
          const auto* weights = static_cast<const float*>(dataPtr);
          for (u32 idx = 0; idx < vertexCount; idx++) {
            vertices[idx].weights = glm::vec4(weights[idx * 4 + 0],
                                              weights[idx * 4 + 1],
                                              weights[idx * 4 + 2],
                                              weights[idx * 4 + 3]);
          }
        }
      }

      // Prepare index data
      const void* indexData = nullptr;
      u64 indexDataSize = 0;
      gfx::IndexType indexType = gfx::IndexType::U16;
      std::vector<u16> widenedIndices;

      if (primitive.indices != -1) {
        const auto& accessor = model.accessors[primitive.indices];
        const auto& bufferView = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[bufferView.buffer];

        const void* indicesPtr =
          &buffer.data[bufferView.byteOffset + accessor.byteOffset];

        newPrim->m_count = accessor.count;
        newPrim->m_offset = 0;

        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
          indexType = gfx::IndexType::U32;
          indexData = indicesPtr;
          indexDataSize = accessor.count * sizeof(u32);

          const auto* indices = static_cast<const u32*>(indicesPtr);
          for (u32 idx = 0; idx + 2 < newPrim->m_count; idx += 3) {
            m_mesh->addTriangleIndices(
              indices[idx], indices[idx + 1], indices[idx + 2]);
          }
        } else if (accessor.componentType ==
                   TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
          indexType = gfx::IndexType::U16;
          indexData = indicesPtr;
          indexDataSize = accessor.count * sizeof(u16);

          const auto* indices = static_cast<const u16*>(indicesPtr);
          for (u32 idx = 0; idx + 2 < newPrim->m_count; idx += 3) {
            m_mesh->addTriangleIndices(
              indices[idx], indices[idx + 1], indices[idx + 2]);
          }
        } else if (accessor.componentType ==
                   TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
          const auto* src = static_cast<const u8*>(indicesPtr);
          widenedIndices.resize(accessor.count);
          for (u32 idx = 0; idx < accessor.count; idx++) {
            widenedIndices[idx] = static_cast<u16>(src[idx]);
          }
          indexType = gfx::IndexType::U16;
          indexData = widenedIndices.data();
          indexDataSize = accessor.count * sizeof(u16);

          for (u32 idx = 0; idx + 2 < newPrim->m_count; idx += 3) {
            m_mesh->addTriangleIndices(src[idx], src[idx + 1], src[idx + 2]);
          }
        }
      } else {
        newPrim->m_count = vertexCount;
      }

      // Set up vertex bindings and attributes for interleaved layout
      std::array<gfx::VertexBinding, 1> bindings = {
        { { 0, sizeof(InterleavedVertex), false } }
      };

      std::array<gfx::VertexAttribute, 6> attributes = { {
        { 0,
          0,
          offsetof(InterleavedVertex, position),
          gfx::PixelFormat::RGB32F },
        { 1, 0, offsetof(InterleavedVertex, normal), gfx::PixelFormat::RGB32F },
        { 2,
          0,
          offsetof(InterleavedVertex, tangent),
          gfx::PixelFormat::RGBA32F },
        { 3,
          0,
          offsetof(InterleavedVertex, texcoord),
          gfx::PixelFormat::RG32F },
        { 4, 0, offsetof(InterleavedVertex, joints), gfx::PixelFormat::RGBA16 },
        { 5,
          0,
          offsetof(InterleavedVertex, weights),
          gfx::PixelFormat::RGBA32F },
      } };

      // Convert primitive mode to topology
      gfx::PrimitiveTopology topology = gfx::gltfModeToTopology(primitive.mode);

      // Create geometry using abstraction
      std::string debugName = m_filename + "_mesh" + std::to_string(meshCount) +
                              "_prim" +
                              std::to_string(p_meshes[meshCount].numPrims - 1);

      auto result =
        resources.createGeometry(vertices.data(),
                                 vertices.size() * sizeof(InterleavedVertex),
                                 indexData,
                                 indexDataSize,
                                 bindings,
                                 attributes,
                                 topology,
                                 vertexCount,
                                 debugName.c_str());

      // Store handles in primitive
      newPrim->m_vaoId = result.vao;
      newPrim->m_vboId = result.vbo;
      newPrim->m_eboId = result.ebo;
      newPrim->m_topology = topology;
      newPrim->m_indexType = indexType;
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
