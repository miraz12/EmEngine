#ifndef ENGINE_PCH_
#define ENGINE_PCH_

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

// STD
#include <array>
#include <cstdint>
#include <map>
#include <memory>
#include <string>

// GLM
#include <glm/ext.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

// STB_IMAGE
#include <stb_image.h>

// Bullet
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <LinearMath/btDefaultMotionState.h>
#include <LinearMath/btVector3.h>
#include <btBulletDynamicsCommon.h>

typedef std::uint64_t u64;
typedef std::int64_t i64;
typedef std::uint32_t u32;
typedef std::int32_t i32;
typedef std::uint16_t u16;
typedef std::int16_t i16;
typedef std::uint8_t u8;
typedef std::int8_t i8;

#endif // ENGINE_PCH_
