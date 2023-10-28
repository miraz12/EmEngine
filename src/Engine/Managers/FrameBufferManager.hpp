#ifndef FRAMEBUFFERMANAGER_H_
#define FRAMEBUFFERMANAGER_H_
#include <Singleton.hpp>
#include <string>
#include <unordered_map>

class FrameBufferManager : public Singleton<FrameBufferManager> {
  friend class Singleton<FrameBufferManager>;

public:
  u32 getFBO(std::string name) { return fbos.at(name); };
  void setFBO(std::string name, u32 &fbo) { fbos[name] = fbo; };
  u32 bindFBO(std::string name);

private:
  FrameBufferManager() = default;
  ~FrameBufferManager() = default;
  std::unordered_map<std::string, u32> fbos;
};

#endif // FRAMEBUFFERMANAGER_H_
