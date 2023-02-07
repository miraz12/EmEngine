#ifndef TEXTURERESOURCE_H_
#define TEXTURERESOURCE_H_

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#include "tiny_gltf.h"

class TextureResource {
 public:
  TextureResource();
  virtual ~TextureResource();
  void LoadTexture(tinygltf::Image image);

 private:
  GLuint texid;
};

#endif  // TEXTURERESOURCE_H_
