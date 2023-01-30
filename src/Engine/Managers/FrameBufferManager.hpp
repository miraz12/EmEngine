#ifndef FRAMEBUFFERMANAGER_H_
#define FRAMEBUFFERMANAGER_H_
#include <string>
#include <unordered_map>

class FrameBufferManager {
public:
  static FrameBufferManager &getInstance() {
    static FrameBufferManager instance;
    return instance;
  }
  FrameBufferManager(FrameBufferManager const &) = delete;
  void operator=(FrameBufferManager const &) = delete;

  unsigned int getFBO(std::string name) { return fbos.at(name); };
  void setFBO(std::string name, unsigned int &fbo) { fbos[name] = fbo; };

private:
  FrameBufferManager() = default;
  std::unordered_map<std::string, unsigned int> fbos;
};

#endif // FRAMEBUFFERMANAGER_H_
