#ifndef TEXTUREMANAGER_H_
#define TEXTUREMANAGER_H_
#include <string>
#include <unordered_map>
#include <vector>

class TextureManager {
public:
  static TextureManager &getInstance() {
    static TextureManager instance;
    return instance;
  }
  TextureManager(TextureManager const &) = delete;
  void operator=(TextureManager const &) = delete;
  // TODO: Make a new function that regenerates textures
  unsigned int loadTexture(unsigned int internalFormat, unsigned int format, unsigned int type,
                           unsigned int width, unsigned int height, unsigned char *data);
  unsigned int loadTexture(std::string name, unsigned int internalFormat, unsigned int format,
                           unsigned int type, unsigned int width, unsigned int height,
                           unsigned char *data);
  void setTexture(std::string name, unsigned int texId);
  void bindTexture(std::string name);
  void deleteTexture(std::string name);

private:
  std::unordered_map<std::string, unsigned int> texIds;
  TextureManager() = default;
};

#endif // TEXTUREMANAGER_H_
