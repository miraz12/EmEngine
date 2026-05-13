#include "AudioSystem.hpp"

#include "ECS/ComponentPool.hpp"
#include "ECS/Components/AudioSourceComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "ECS/ECSManager.hpp"
#include "ECS/Systems/CameraSystem.hpp"

#include <AL/al.h>
#include <AL/alc.h>
#include <dr_mp3.h>
#include <dr_wav.h>

// stb_vorbis is compiled as a separate TU, just need the header
#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

namespace {

#ifndef NDEBUG
void
checkALError(const char* context)
{
  ALenum err = alGetError();
  if (err != AL_NO_ERROR) {
    std::cerr << "[AudioSystem] OpenAL error at " << context << ": 0x"
              << std::hex << err << std::dec << std::endl;
  }
}
#define AL_CHECK(msg) checkALError(msg)
#else
#define AL_CHECK(msg) ((void)0)
#endif

} // namespace

void
AudioSystem::initialize(ECSManager& ecsManager)
{
  System::initialize(ecsManager);

  m_device = alcOpenDevice(nullptr);
  if (!m_device) {
    std::cerr << "[AudioSystem] Failed to open audio device" << std::endl;
    return;
  }

  m_context = alcCreateContext(m_device, nullptr);
  if (!m_context || !alcMakeContextCurrent(m_context)) {
    std::cerr << "[AudioSystem] Failed to create audio context" << std::endl;
    if (m_context)
      alcDestroyContext(m_context);
    alcCloseDevice(m_device);
    m_device = nullptr;
    m_context = nullptr;
    return;
  }

  // Pre-allocate source pool
  m_allSources.resize(kMaxSources);
  alGenSources(static_cast<ALsizei>(kMaxSources), m_allSources.data());
  AL_CHECK("alGenSources");

  m_availableSources = m_allSources;

  // Good default for 3D game audio
  alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
  AL_CHECK("alDistanceModel");

  m_initialized = true;

  const ALCchar* deviceName = alcGetString(m_device, ALC_DEVICE_SPECIFIER);
  std::cout << "[AudioSystem] Initialized with device: "
            << (deviceName ? deviceName : "unknown") << std::endl;
}

void
AudioSystem::update(float /* dt */)
{
  if (!m_initialized)
    return;

  // Sync listener to main camera
  auto* cam = CameraSystem::getInstance().getMainCameraComponent();
  if (cam) {
    alListener3f(
      AL_POSITION, cam->m_position.x, cam->m_position.y, cam->m_position.z);
    ALfloat orientation[6] = { cam->m_front.x, cam->m_front.y, cam->m_front.z,
                               cam->m_up.x,    cam->m_up.y,    cam->m_up.z };
    alListenerfv(AL_ORIENTATION, orientation);
  }

  alListenerf(AL_GAIN, m_masterVolume);

  // Iterate audio components
  auto* pool = m_manager->getPool<AudioSourceComponent>();
  if (!pool)
    return;

  for (size_t i = 0; i < pool->size(); ++i) {
    auto& audio = (*pool)[i];
    Entity entity = pool->entityAt(i);

    // Handle play request
    if (audio.play) {
      audio.play = false;

      // Load buffer if not cached
      u32 bufferId = loadAudioClip(audio.clipPath);
      if (bufferId == 0)
        continue;

      // Acquire a source if we don't have one
      if (audio.internalSourceId == 0) {
        audio.internalSourceId = acquireSource();
        if (audio.internalSourceId == 0)
          continue; // No sources available
      }

      ALuint src = audio.internalSourceId;
      alSourcei(src, AL_BUFFER, static_cast<ALint>(bufferId));
      alSourcef(src, AL_GAIN, audio.volume);
      alSourcef(src, AL_PITCH, audio.pitch);
      alSourcei(src, AL_LOOPING, audio.loop ? AL_TRUE : AL_FALSE);

      if (audio.is3D) {
        alSourcei(src, AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef(src, AL_REFERENCE_DISTANCE, audio.minDistance);
        alSourcef(src, AL_MAX_DISTANCE, audio.maxDistance);
        alSourcef(src, AL_ROLLOFF_FACTOR, audio.rolloffFactor);

        // Set initial position from entity
        auto* pos = m_manager->getComponent<PositionComponent>(entity);
        if (pos) {
          alSource3f(src,
                     AL_POSITION,
                     pos->position.x,
                     pos->position.y,
                     pos->position.z);
        }
      } else {
        // 2D: play relative to listener at origin
        alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);
        alSource3f(src, AL_POSITION, 0.0f, 0.0f, 0.0f);
      }

      alSourcePlay(src);
      AL_CHECK("alSourcePlay");
      audio.isPlaying = true;
    }

    // Handle stop request
    if (audio.stop) {
      audio.stop = false;
      if (audio.internalSourceId != 0) {
        alSourceStop(audio.internalSourceId);
        releaseSource(audio.internalSourceId);
        audio.internalSourceId = 0;
        audio.isPlaying = false;
      }
    }

    // Update active sources
    if (audio.internalSourceId != 0) {
      ALuint src = audio.internalSourceId;

      // Sync parameters
      alSourcef(src, AL_GAIN, audio.volume);
      alSourcef(src, AL_PITCH, audio.pitch);

      // Sync 3D position
      if (audio.is3D) {
        auto* pos = m_manager->getComponent<PositionComponent>(entity);
        if (pos) {
          alSource3f(src,
                     AL_POSITION,
                     pos->position.x,
                     pos->position.y,
                     pos->position.z);
        }
      }

      // Check if a non-looping sound has finished
      ALint state;
      alGetSourcei(src, AL_SOURCE_STATE, &state);
      if (state == AL_STOPPED) {
        releaseSource(src);
        audio.internalSourceId = 0;
        audio.isPlaying = false;
      }
    }
  }
}

u32
AudioSystem::loadAudioClip(const std::string& path)
{
  auto it = m_bufferCache.find(path);
  if (it != m_bufferCache.end())
    return it->second;

  // Determine format from extension
  std::string ext;
  auto dot = path.rfind('.');
  if (dot != std::string::npos)
    ext = path.substr(dot);

  i16* pcmData = nullptr;
  u32 channels = 0;
  u32 sampleRate = 0;
  u64 totalFrames = 0;

  if (ext == ".wav") {
    drwav wav;
    if (!drwav_init_file(&wav, path.c_str(), nullptr)) {
      std::cerr << "[AudioSystem] Failed to load WAV: " << path << std::endl;
      return 0;
    }
    totalFrames = wav.totalPCMFrameCount;
    channels = wav.channels;
    sampleRate = wav.sampleRate;
    pcmData = static_cast<i16*>(
      malloc(static_cast<size_t>(totalFrames * channels) * sizeof(i16)));
    drwav_read_pcm_frames_s16(&wav, totalFrames, pcmData);
    drwav_uninit(&wav);
  } else if (ext == ".mp3") {
    drmp3_config config;
    drmp3_uint64 mp3Frames;
    pcmData = drmp3_open_file_and_read_pcm_frames_s16(
      path.c_str(), &config, &mp3Frames, nullptr);
    if (!pcmData) {
      std::cerr << "[AudioSystem] Failed to load MP3: " << path << std::endl;
      return 0;
    }
    totalFrames = mp3Frames;
    channels = config.channels;
    sampleRate = config.sampleRate;
  } else if (ext == ".ogg") {
    int vorbisChannels, vorbisSampleRate;
    short* vorbisData;
    int samples = stb_vorbis_decode_filename(
      path.c_str(), &vorbisChannels, &vorbisSampleRate, &vorbisData);
    if (samples <= 0) {
      std::cerr << "[AudioSystem] Failed to load OGG: " << path << std::endl;
      return 0;
    }
    pcmData = vorbisData;
    totalFrames = static_cast<u64>(samples);
    channels = static_cast<u32>(vorbisChannels);
    sampleRate = static_cast<u32>(vorbisSampleRate);
  } else {
    std::cerr << "[AudioSystem] Unsupported audio format: " << ext << std::endl;
    return 0;
  }

  ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

  ALuint buffer;
  alGenBuffers(1, &buffer);
  alBufferData(buffer,
               format,
               pcmData,
               static_cast<ALsizei>(totalFrames * channels * sizeof(i16)),
               static_cast<ALsizei>(sampleRate));
  AL_CHECK("alBufferData");

  free(pcmData);

  m_bufferCache[path] = buffer;
  return buffer;
}

void
AudioSystem::unloadAudioClip(const std::string& path)
{
  auto it = m_bufferCache.find(path);
  if (it != m_bufferCache.end()) {
    alDeleteBuffers(1, &it->second);
    m_bufferCache.erase(it);
  }
}

u32
AudioSystem::acquireSource()
{
  if (m_availableSources.empty()) {
    std::cerr << "[AudioSystem] No available audio sources" << std::endl;
    return 0;
  }
  u32 src = m_availableSources.back();
  m_availableSources.pop_back();
  return src;
}

void
AudioSystem::releaseSource(u32 sourceId)
{
  if (sourceId == 0)
    return;

  alSourceStop(sourceId);
  alSourcei(sourceId, AL_BUFFER, 0); // Detach buffer
  m_availableSources.push_back(sourceId);
}

void
AudioSystem::setMasterVolume(float volume)
{
  m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
}

void
AudioSystem::shutdown()
{
  if (!m_initialized)
    return;

  // Stop and delete all sources
  if (!m_allSources.empty()) {
    alSourceStopv(static_cast<ALsizei>(m_allSources.size()),
                  m_allSources.data());
    alDeleteSources(static_cast<ALsizei>(m_allSources.size()),
                    m_allSources.data());
  }
  m_allSources.clear();
  m_availableSources.clear();

  // Delete all cached buffers
  for (auto& [path, bufferId] : m_bufferCache) {
    alDeleteBuffers(1, &bufferId);
  }
  m_bufferCache.clear();

  // Destroy context and close device
  alcMakeContextCurrent(nullptr);
  if (m_context)
    alcDestroyContext(m_context);
  if (m_device)
    alcCloseDevice(m_device);

  m_context = nullptr;
  m_device = nullptr;
  m_initialized = false;

  std::cout << "[AudioSystem] Shutdown complete" << std::endl;
}

AudioSystem::~AudioSystem()
{
  shutdown();
}
