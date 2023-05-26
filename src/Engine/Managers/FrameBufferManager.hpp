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

  uint32_t getFBO(std::string name) { return fbos.at(name); };
  void setFBO(std::string name, uint32_t &fbo) { fbos[name] = fbo; };
  uint32_t bindFBO(std::string name);

private:
  FrameBufferManager() = default;
  std::unordered_map<std::string, uint32_t> fbos;
};

#endif // FRAMEBUFFERMANAGER_H_
