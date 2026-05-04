#ifndef DEBUGDRAWER_H_
#define DEBUGDRAWER_H_

#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/Handle.hpp>

class DebugDrawer
{
public:
  DebugDrawer();
  ~DebugDrawer();

  void initialize();
  void shutdown();

  void drawLine(const glm::vec3& from,
                const glm::vec3& to,
                const glm::vec3& color);

  void renderAndFlush(gfx::ShaderId shader = {});

private:
  struct Line
  {
    glm::vec3 from;
    glm::vec3 to;
    glm::vec3 color;
  };

  gfx::VertexArrayId m_vao{};
  gfx::BufferId m_vbo{};
  u64 m_vboCapacity{ 0 };
  bool m_initialized{ false };

  static constexpr u64 kInitialCapacity =
    1000 * 2 * 6 * sizeof(float); // 1000 lines

public:
  std::vector<Line> lines;
};

#endif // DEBUGDRAWER_H_
