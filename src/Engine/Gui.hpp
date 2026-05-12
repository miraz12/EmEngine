#ifndef GUI_H_
#define GUI_H_

#ifndef NDEBUG

#include "ECS/Components/CameraComponent.hpp"

#include <string>
#include <vector>

class Profiler;

class GUI
{

public:
  GUI() = default;
  ~GUI() = default;
  void renderGUI(Profiler& profiler);

private:
  void editTransform(std::shared_ptr<CameraComponent> camera,
                     glm::vec3& pos,
                     glm::vec3& rot,
                     glm::vec3& scale);
  void drawSceneHierarchy();
  void drawInspector();
  void drawSettingsWindow(Profiler& profiler);
  void drawProfilerWindow(Profiler& profiler);
  void drawCreateEntityPopup();
  void scanAvailableModels();

  // Entity creation popup state
  bool m_showCreatePopup{ false };
  char m_newEntityName[128]{ "new_entity" };
  bool m_createWithPosition{ true };
  bool m_createWithGraphics{ false };
  bool m_createWithPhysics{ false };
  int m_createGraphicsType{ 3 }; // CUBE
  int m_createPhysicsShape{ 0 }; // BOX
  float m_createMass{ 0.0f };
  int m_selectedModel{ -1 };
  char m_createHeightmapPath[256]{ "" };

  // Available model paths (relative to resources/Models/)
  std::vector<std::string> m_availableModels;
  bool m_modelsScanned{ false };

  // Hierarchy filter
  char m_entityFilter[128]{ "" };
};

#endif // !NDEBUG

#endif // GUI_H_
