#ifndef DEBUGDRAWER_H_
#define DEBUGDRAWER_H_

/* vertex data is passed as input to this shader
 * ourColor is passed as input to the to the fragment shader. */
class DebugDrawer : public btIDebugDraw
{

public:
  DebugDrawer();
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
  void renderAndFlush();

  void setDebugMode(i32 debugMode) override { m_debugMode = debugMode; }
  i32 getDebugMode() const override { return m_debugMode; }

private:
  struct Line
  {
    btVector3 from;
    btVector3 to;
  };
  u32 m_debugMode = DBG_DrawWireframe;
  std::vector<Line> lines;
};
#endif // DEBUGDRAWER_H_
