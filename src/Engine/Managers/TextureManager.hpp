#ifndef TEXTUREMANAGER_H_
#define TEXTUREMANAGER_H_
#include <vector>

class TextureManager {
public:
  static TextureManager &getInstance() {
    static TextureManager instance;
    return instance;
  }
  TextureManager(TextureManager const &) = delete;
  void operator=(TextureManager const &) = delete;
  void loadTexture(unsigned int format, unsigned int type, unsigned int width, unsigned int height,
                   unsigned char *data);
  void bindTexture(unsigned int texIdx);

private:
  std::vector<unsigned int> texIds;
  TextureManager() = default;
};

#endif // TEXTUREMANAGER_H_
