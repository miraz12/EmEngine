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
typedef std::uint64_t u64;
typedef std::int64_t i64;
typedef std::uint32_t u32;
typedef std::int32_t i32;
typedef std::uint16_t u16;
typedef std::int16_t i16;
typedef std::uint8_t u8;
typedef std::int8_t i8;
// Undef int to only allow the ones defined here
// #define int undefined

// Defines
#define PI 3.14159f

#define DEG2RAD PI / 180.f
#define RAD2DEG 180.f / PI

#endif // ENGINE_PCH_
