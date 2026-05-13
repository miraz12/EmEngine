#ifndef AUDIOSOURCECOMPONENT_H_
#define AUDIOSOURCECOMPONENT_H_

#include <string>

struct AudioSourceComponent
{
  // Audio clip to play (path relative to resources/)
  std::string clipPath;

  // Playback control (set by game layer, consumed by AudioSystem)
  bool play{ false };
  bool stop{ false };
  bool loop{ false };
  bool is3D{ true }; // true = positional audio, false = 2D (music/UI)

  // Audio parameters
  float volume{ 1.0f };
  float pitch{ 1.0f };
  float minDistance{ 1.0f };
  float maxDistance{ 50.0f };
  float rolloffFactor{ 1.0f };

  // Internal state (managed by AudioSystem, read-only for game layer)
  u32 internalSourceId{ 0 };
  bool isPlaying{ false };

  AudioSourceComponent() = default;
  explicit AudioSourceComponent(std::string path)
    : clipPath(std::move(path))
  {
  }

  AudioSourceComponent(AudioSourceComponent&& other) noexcept
    : clipPath(std::move(other.clipPath))
    , play(other.play)
    , stop(other.stop)
    , loop(other.loop)
    , is3D(other.is3D)
    , volume(other.volume)
    , pitch(other.pitch)
    , minDistance(other.minDistance)
    , maxDistance(other.maxDistance)
    , rolloffFactor(other.rolloffFactor)
    , internalSourceId(other.internalSourceId)
    , isPlaying(other.isPlaying)
  {
    other.internalSourceId = 0; // Invalidate source so its dtor is a no-op
    other.isPlaying = false;
  }

  AudioSourceComponent& operator=(AudioSourceComponent&& other) noexcept;
  ~AudioSourceComponent();
};

#endif // AUDIOSOURCECOMPONENT_H_
