#ifndef TEXTUREMANAGER_H_
#define TEXTUREMANAGER_H_
#include <Singleton.hpp>
#include <string>
#include <unordered_map>
#include <vector>

class TextureManager : public Singleton<TextureManager>{
    friend class Singleton<TextureManager>;
public:

  // TODO: Make a new function that regenerates textures
  uint32_t loadTexture(uint32_t internalFormat, uint32_t format, uint32_t type, uint32_t width,
                       uint32_t height, unsigned char *data);
  uint32_t loadTexture(std::string name, uint32_t internalFormat, uint32_t format, uint32_t type,
                       uint32_t width, uint32_t height, unsigned char *data);
  void setTexture(std::string name, uint32_t texId);
  uint32_t bindTexture(std::string name);
  void bindCubeTexture(std::string name);
  void deleteTexture(std::string name);

private:
  std::unordered_map<std::string, uint32_t> texIds;
  TextureManager() = default;
};

#endif // TEXTUREMANAGER_H_
