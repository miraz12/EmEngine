#ifndef AUDIOSYSTEM_H_
#define AUDIOSYSTEM_H_

#include "System.hpp"
#include <string>
#include <unordered_map>
#include <vector>

// OpenAL types (Emscripten's headers use different struct names for the typedefs)
#include <AL/al.h>
#include <AL/alc.h>

class AudioSystem
  : public System
  , public Singleton<AudioSystem>
{
  friend class Singleton<AudioSystem>;

public:
  void initialize(ECSManager& ecsManager) override;
  void update(float dt) override;
  void shutdown();

  // Buffer management (load/cache audio files, returns OpenAL buffer ID)
  u32 loadAudioClip(const std::string& path);
  void unloadAudioClip(const std::string& path);

  // Source pool (called by AudioSourceComponent lifecycle)
  u32 acquireSource();
  void releaseSource(u32 sourceId);

  void setMasterVolume(float volume);
  float getMasterVolume() const { return m_masterVolume; }

private:
  AudioSystem() = default;
  ~AudioSystem() override;

  ALCdevice* m_device{ nullptr };
  ALCcontext* m_context{ nullptr };

  // File path -> OpenAL buffer ID
  std::unordered_map<std::string, u32> m_bufferCache;

  // Pre-allocated source pool
  std::vector<u32> m_availableSources;
  std::vector<u32> m_allSources;
  static constexpr u32 kMaxSources = 64;

  float m_masterVolume{ 1.0f };
  bool m_initialized{ false };
};

#endif // AUDIOSYSTEM_H_
