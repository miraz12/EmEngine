#ifndef FRAMEBUFFERMANAGER_H_
#define FRAMEBUFFERMANAGER_H_
#include <Singleton.hpp>
#include <string>
#include <unordered_map>

class FrameBufferManager : public Singleton<FrameBufferManager>{
    friend class Singleton<FrameBufferManager>;
public:
  uint32_t getFBO(std::string name) { return fbos.at(name); };
  void setFBO(std::string name, uint32_t &fbo) { fbos[name] = fbo; };
  uint32_t bindFBO(std::string name);

private:
  FrameBufferManager() = default;
  ~FrameBufferManager() = default;
  std::unordered_map<std::string, uint32_t> fbos;
};

#endif // FRAMEBUFFERMANAGER_H_
