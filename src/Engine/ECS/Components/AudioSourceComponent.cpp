#include "AudioSourceComponent.hpp"

#include <ECS/Systems/AudioSystem.hpp>

AudioSourceComponent&
AudioSourceComponent::operator=(AudioSourceComponent&& other) noexcept
{
  if (this != &other) {
    if (internalSourceId != 0) {
      AudioSystem::getInstance().releaseSource(internalSourceId);
    }
    clipPath = std::move(other.clipPath);
    play = other.play;
    stop = other.stop;
    loop = other.loop;
    is3D = other.is3D;
    volume = other.volume;
    pitch = other.pitch;
    minDistance = other.minDistance;
    maxDistance = other.maxDistance;
    rolloffFactor = other.rolloffFactor;
    internalSourceId = other.internalSourceId;
    isPlaying = other.isPlaying;
    other.internalSourceId = 0;
    other.isPlaying = false;
  }
  return *this;
}

AudioSourceComponent::~AudioSourceComponent()
{
  if (internalSourceId != 0) {
    AudioSystem::getInstance().releaseSource(internalSourceId);
  }
}
