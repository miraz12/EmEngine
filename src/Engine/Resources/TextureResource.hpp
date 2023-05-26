#ifndef TEXTURERESOURCE_H_
#define TEXTURERESOURCE_H_

#include "tiny_gltf.h"

class TextureResource {
public:
  TextureResource();
  virtual ~TextureResource();
  void LoadTexture(tinygltf::Image image);

private:
  GLuint texid;
};

#endif // TEXTURERESOURCE_H_
