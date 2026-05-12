#ifndef PROFILER_H_
#define PROFILER_H_

#ifndef NDEBUG

#include <array>
#include <chrono>
#include <string_view>

struct ProfilerConfig
{
  static constexpr size_t kHistorySize = 256;
  static constexpr size_t kMaxSections = 16;
  static constexpr float kSmoothingFactor =
    0.05f; // EMA alpha (lower = smoother)
};

struct RingBuffer
{
  std::array<float, ProfilerConfig::kHistorySize> data{};
  size_t head{ 0 };
  size_t count{ 0 };

  void push(float value)
  {
    data[head] = value;
    head = (head + 1) % ProfilerConfig::kHistorySize;
    if (count < ProfilerConfig::kHistorySize)
      ++count;
  }

  float operator[](size_t i) const
  {
    size_t idx = (head + ProfilerConfig::kHistorySize - count + i) %
                 ProfilerConfig::kHistorySize;
    return data[idx];
  }

  static float getter(void* ringBuf, int index)
  {
    auto* buf = static_cast<RingBuffer*>(ringBuf);
    return (*buf)[static_cast<size_t>(index)];
  }
};

enum class SectionCategory : uint8_t
{
  kSystem,
  kRenderPass
};

struct TimerEntry
{
  std::string_view name;
  float durationMs{ 0.0f };
  SectionCategory category{ SectionCategory::kSystem };
};

class Profiler
{
public:
  using Clock = std::chrono::high_resolution_clock;
  using TimePoint = Clock::time_point;

  void beginFrame() { m_frameStart = Clock::now(); }

  void endFrame()
  {
    constexpr float alpha = ProfilerConfig::kSmoothingFactor;
    auto now = Clock::now();
    float rawFrameMs = msFrom(m_frameStart, now);
    float rawFps = (rawFrameMs > 0.0f) ? 1000.0f / rawFrameMs : 0.0f;

    m_smoothedFrameMs = lerp(m_smoothedFrameMs, rawFrameMs, alpha);
    m_smoothedFps = lerp(m_smoothedFps, rawFps, alpha);

    m_fpsHistory.push(rawFps);
    m_frameTimeHistory.push(rawFrameMs);

    // Smooth section durations
    for (size_t idx = 0; idx < m_sectionCount; ++idx) {
      m_smoothedSections[idx].name = m_sections[idx].name;
      m_smoothedSections[idx].category = m_sections[idx].category;
      m_smoothedSections[idx].durationMs = lerp(
        m_smoothedSections[idx].durationMs, m_sections[idx].durationMs, alpha);
    }

    // Smooth phase durations
    for (size_t idx = 0; idx < kNumPhases; ++idx) {
      m_smoothedPhaseDurations[idx] =
        lerp(m_smoothedPhaseDurations[idx], m_phaseDurations[idx], alpha);
    }
  }

  void resetSections() { m_sectionCount = 0; }

  void beginSection(std::string_view name, SectionCategory cat)
  {
    if (m_sectionCount < ProfilerConfig::kMaxSections) {
      m_sections[m_sectionCount].name = name;
      m_sections[m_sectionCount].category = cat;
      m_sectionStarts[m_sectionCount] = Clock::now();
    }
  }

  void endSection()
  {
    if (m_sectionCount < ProfilerConfig::kMaxSections) {
      m_sections[m_sectionCount].durationMs =
        msFrom(m_sectionStarts[m_sectionCount], Clock::now());
      ++m_sectionCount;
    }
  }

  static constexpr size_t kPhaseECS = 0;
  static constexpr size_t kPhaseUI = 1;
  static constexpr size_t kPhaseSwap = 2;
  static constexpr size_t kNumPhases = 3;

  void beginPhase(size_t idx) { m_phaseStarts[idx] = Clock::now(); }
  void endPhase(size_t idx)
  {
    m_phaseDurations[idx] = msFrom(m_phaseStarts[idx], Clock::now());
  }

  float getFps() const { return m_smoothedFps; }
  float getFrameTimeMs() const { return m_smoothedFrameMs; }
  size_t getSectionCount() const { return m_sectionCount; }
  const TimerEntry& getSection(size_t i) const { return m_smoothedSections[i]; }
  float getPhaseMs(size_t i) const { return m_smoothedPhaseDurations[i]; }

  RingBuffer& fpsHistory()
  {
    return m_fpsHistory;
  }
  RingBuffer& frameTimeHistory()
  {
    return m_frameTimeHistory;
  }

  bool visible{ false };

private:
  static float msFrom(TimePoint a, TimePoint b)
  {
    return std::chrono::duration<float, std::milli>(b - a).count();
  }

  static float lerp(float current, float target, float alpha)
  {
    return current + alpha * (target - current);
  }

  TimePoint m_frameStart;

  RingBuffer m_fpsHistory;
  RingBuffer m_frameTimeHistory;

  // Raw per-frame values
  std::array<TimerEntry, ProfilerConfig::kMaxSections> m_sections;
  std::array<TimePoint, ProfilerConfig::kMaxSections> m_sectionStarts;
  size_t m_sectionCount{ 0 };
  std::array<TimePoint, kNumPhases> m_phaseStarts;
  std::array<float, kNumPhases> m_phaseDurations{};

  // Smoothed values (EMA)
  float m_smoothedFps{ 0.0f };
  float m_smoothedFrameMs{ 0.0f };
  std::array<TimerEntry, ProfilerConfig::kMaxSections> m_smoothedSections;
  std::array<float, kNumPhases> m_smoothedPhaseDurations{};
};

#endif // !NDEBUG

#endif // PROFILER_H_
