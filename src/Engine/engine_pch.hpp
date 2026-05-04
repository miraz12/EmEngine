#ifndef ENGINE_PCH_
#define ENGINE_PCH_

#include "Singleton.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#define GLM_FORCE_INTRINSICS
#define GLM_FORCE_ALIGNED_GENTYPES
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
#include <math.h>
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

// Jolt Physics (minimal — heavy includes in .cpp files only)
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

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

#define DEG2RAD (M_PI / 180.f)
#define RAD2DEG (180.f / M_PI)

#endif // ENGINE_PCH_
