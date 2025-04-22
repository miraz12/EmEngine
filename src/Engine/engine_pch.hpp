#ifndef ENGINE_PCH_
#define ENGINE_PCH_

#include "Singleton.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#define GLM_ENABLE_EXPERIMENTAL

// STD
#include <algorithm>
#include <array>
#include <bitset>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <ostream>
#include <queue>
#include <random>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

// GLM
#include "glm/geometric.hpp"
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

// Bullet
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btConvexShape.h>
#include <BulletCollision/CollisionShapes/btConvexTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btTriangleMesh.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btDefaultMotionState.h>
#include <LinearMath/btVector3.h>
#include <btBulletDynamicsCommon.h>

// STB_IMAGE
#include <stb_image.h>

// GLFW
#include <GLFW/glfw3.h>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <imgui_internal.h>

#include <tiny_gltf.h>

// ImGuizmo
#include <ImGuizmo.h>

// yaml-cpp
#include <yaml-cpp/yaml.h>

// Types
using u64 = std::uint64_t;
using i64 = std::int64_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
using u16 = std::uint16_t;
using i16 = std::int16_t;
using u8 = std::uint8_t;
using i8 = std::int8_t;

constexpr std::size_t MAX_COMPONENTS = 32;
constexpr std::size_t MAX_ENTITIES = 1000;
using ComponentType = std::type_index;
using Entity = std::size_t;
using Signature = std::bitset<MAX_COMPONENTS>;

// Undef int to only allow the ones defined here
// #define int undefined

// Defines
constexpr float PI = 3.14159f;

#define DEG2RAD PI / 180.f
#define RAD2DEG 180.f / PI

static void
checkGLError(const char* operation)
{
  GLenum error;
  while ((error = glGetError()) != GL_NO_ERROR) {
    std::cout << "GL error after " << operation << ": 0x" << std::hex << error
              << std::dec << std::endl;
  }
}

#endif // ENGINE_PCH_
