#include "Gui.hpp"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
static const float identityMatrix[16] = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f,
                                         0.f, 0.f, 0.f, 0.f, 1.f, 0.f,
                                         0.f, 0.f, 0.f, 1.f};
void GUI::renderGUI() {

  // Start the Dear ImGui frame

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin("Settings", 0, ImGuiWindowFlags_AlwaysAutoResize);

  if (ImGui::CollapsingHeader("Lights")) {
    ImGui::SliderFloat3("Direction", glm::value_ptr(m_game->dirLightDir), -1.0f,
                        1.0f);
    ImGui::SliderFloat("Ambient", &m_game->dirLightAmbient, 0.0f, 2.0f);
    ImGui::ColorEdit3("Color", glm::value_ptr(m_game->dirLightColor));
  }

  if (ImGui::CollapsingHeader("Physics")) {
    ImGui::Checkbox("Enabled", &ECSManager::getInstance().simulatePhysics);
    Entity en = m_game->m_ECSManager.getPickedEntity();
    if (m_game->m_ECSManager.getEntitySelected()) {
      ImGui::Text("Selected entity: %lu", en);
      glm::vec3 pos =
          m_game->m_ECSManager.getComponent<PositionComponent>(en)->position;
      ImGui::Text("Position: X: %f Y: %f Z: %f ", pos.x, pos.y, pos.z);
      Camera &cam = ECSManager::getInstance().getCamera();
      // ImGuizmo::Manipulate(glm::value_ptr(cam.getViewMatrix()),
      //                      glm::value_ptr(cam.getProjectionMatrix()),
      //                      mCurrentGizmoOperation, mCurrentGizmoMode,
      //                      glm::value_ptr(pos), NULL, NULL, NULL, NULL);
    }
  }

  static i32 offset = 0;
  offset = (offset + 1) % 50;
  if (ImGui::CollapsingHeader("Debug")) {
    // ImGui::PlotLines("FPS", fpsArray, 50, offset, nullptr, 0, 60,
    //                  ImVec2(0, 80.f));

    const std::vector<std::string> debugNamesInputs = {
        "none",     "Base color", "Normal",   "Occlusion",
        "Emissive", "Metallic",   "Roughness"};
    std::vector<const char *> charitems;
    charitems.reserve(debugNamesInputs.size());
    for (size_t i = 0; i < debugNamesInputs.size(); i++) {
      charitems.push_back(debugNamesInputs[i].c_str());
    }
    ImGui::Combo("views", &ECSManager::getInstance().debugView, &charitems[0],
                 7, 7);
  }
  ImGui::End();

  renderGizmos();

  // Rendering
  ImGui::Render();
}

void GUI::renderGizmos() {

  ImGuiIO &io = ImGui::GetIO();
  Camera &cam = ECSManager::getInstance().getCamera();

  ImGuizmo::SetOrthographic(false);
  ImGuizmo::BeginFrame();

  float viewManipulateRight = io.DisplaySize.x;
  float viewManipulateTop = 0;

  ImGuizmo::DrawGrid(glm::value_ptr(cam.getViewMatrix()),
                     glm::value_ptr(cam.getProjectionMatrix()), identityMatrix,
                     1000.f);

  ImGuizmo::ViewManipulate(glm::value_ptr(cam.getViewMatrix()), camDistance,
                           ImVec2(viewManipulateRight - 128, viewManipulateTop),
                           ImVec2(128, 128), 0x10101010);
}