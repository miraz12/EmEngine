#ifndef DEBUGDRAWER_H_
#define DEBUGDRAWER_H_

#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/Handle.hpp>

/* vertex data is passed as input to this shader
 * ourColor is passed as input to the to the fragment shader. */
class DebugDrawer : public btIDebugDraw
{

public:
  DebugDrawer();
  ~DebugDrawer();

  /// Initialize persistent graphics resources (call after GraphicsDevice init)
  void initialize();

  /// Release graphics resources
  void shutdown();

  void drawLine(const btVector3& from,
                const btVector3& to,
                const btVector3& color) override;
  void drawContactPoint(const btVector3& pointOnB,
                        const btVector3& normalOnB,
                        btScalar distance,
                        i32 lifeTime,
                        const btVector3& color) override;
  void reportErrorWarning(const char* warningString) override;
  void draw3dText(const btVector3& location, const char* textString) override;
  void renderAndFlush(gfx::ShaderId shader = {});

  void setDebugMode(i32 debugMode) override { m_debugMode = debugMode; }
  i32 getDebugMode() const override { return m_debugMode; }

private:
  struct Line
  {
    btVector3 from;
    btVector3 to;
    btVector3 color;
  };
  u32 m_debugMode = DBG_DrawWireframe | DBG_FastWireframe;

  // Persistent graphics resources (avoid per-frame allocation)
  gfx::VertexArrayId m_vao{};
  gfx::BufferId m_vbo{};
  u64 m_vboCapacity{ 0 }; // Current buffer capacity in bytes
  bool m_initialized{ false };

  static constexpr u64 kInitialCapacity =
    1000 * 2 * 6 * sizeof(float); // 1000 lines

public:
  std::vector<Line> lines;
};
#endif // DEBUGDRAWER_H_
