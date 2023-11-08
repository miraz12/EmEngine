#ifndef TEXTUREMANAGER_H_
#define TEXTUREMANAGER_H_
#include <Singleton.hpp>

class TextureManager : public Singleton<TextureManager> {
  friend class Singleton<TextureManager>;

public:
  // TODO: Make a new function that regenerates textures
  u32 loadTexture(u32 internalFormat, u32 format, u32 type, u32 width,
                       u32 height, unsigned char *data);
  u32 loadTexture(std::string name, u32 internalFormat, u32 format, u32 type,
                       u32 width, u32 height, unsigned char *data);
  void setTexture(std::string name, u32 texId, u32 type = GL_TEXTURE_2D);
  u32 bindTexture(std::string name);
  u32 bindActivateTexture(std::string name, u32 pos);
  void bindCubeTexture(std::string name);
  void bindActivateCubeTexture(std::string name, u32 pos);
  void deleteTexture(std::string name);

private:
  struct Texture {
    u32 id;
    u32 type;
  };
  std::unordered_map<std::string, Texture> texIds;
  TextureManager() = default;
};

#endif // TEXTUREMANAGER_H_
