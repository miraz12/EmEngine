#include "Gui.hpp"


#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
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
  // Rendering
  ImGui::Render();
}

void GUI::editTransform(float *cameraView, float *cameraProjection, float *matrix,
                   bool editTransformDecomposition) {
  static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
  static bool useSnap = false;
  static float snap[3] = {1.f, 1.f, 1.f};
  static float bounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
  static float boundsSnap[] = {0.1f, 0.1f, 0.1f};
  static bool boundSizing = false;
  static bool boundSizingSnap = false;

  if (editTransformDecomposition) {
    if (ImGui::IsKeyPressed(ImGuiKey_T))
      mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E))
      mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R)) // r Key
      mCurrentGizmoOperation = ImGuizmo::SCALE;
    if (ImGui::RadioButton("Translate",
                           mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
      mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate",
                           mCurrentGizmoOperation == ImGuizmo::ROTATE))
      mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
      mCurrentGizmoOperation = ImGuizmo::SCALE;
    if (ImGui::RadioButton("Universal",
                           mCurrentGizmoOperation == ImGuizmo::UNIVERSAL))
      mCurrentGizmoOperation = ImGuizmo::UNIVERSAL;
    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation,
                                          matrixRotation, matrixScale);
    ImGui::InputFloat3("Tr", matrixTranslation);
    ImGui::InputFloat3("Rt", matrixRotation);
    ImGui::InputFloat3("Sc", matrixScale);
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation,
                                            matrixScale, matrix);

    if (mCurrentGizmoOperation != ImGuizmo::SCALE) {
      if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
        mCurrentGizmoMode = ImGuizmo::LOCAL;
      ImGui::SameLine();
      if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
        mCurrentGizmoMode = ImGuizmo::WORLD;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_S))
      useSnap = !useSnap;
    ImGui::Checkbox("##UseSnap", &useSnap);
    ImGui::SameLine();

    switch (mCurrentGizmoOperation) {
    case ImGuizmo::TRANSLATE:
      ImGui::InputFloat3("Snap", &snap[0]);
      break;
    case ImGuizmo::ROTATE:
      ImGui::InputFloat("Angle Snap", &snap[0]);
      break;
    case ImGuizmo::SCALE:
      ImGui::InputFloat("Scale Snap", &snap[0]);
      break;
    }
    ImGui::Checkbox("Bound Sizing", &boundSizing);
    if (boundSizing) {
      ImGui::PushID(3);
      ImGui::Checkbox("##BoundSizing", &boundSizingSnap);
      ImGui::SameLine();
      ImGui::InputFloat3("Snap", boundsSnap);
      ImGui::PopID();
    }
  }

  ImGuiIO &io = ImGui::GetIO();
  float viewManipulateRight = io.DisplaySize.x;
  float viewManipulateTop = 0;
  static ImGuiWindowFlags gizmoWindowFlags = 0;
  if (useWindow) {
    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(400, 20), ImGuiCond_Appearing);
    ImGui::PushStyleColor(ImGuiCol_WindowBg,
                          (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
    ImGui::Begin("Gizmo", 0, gizmoWindowFlags);
    ImGuizmo::SetDrawlist();
    float windowWidth = (float)ImGui::GetWindowWidth();
    float windowHeight = (float)ImGui::GetWindowHeight();
    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y,
                      windowWidth, windowHeight);
    viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
    viewManipulateTop = ImGui::GetWindowPos().y;
    ImGuiWindow *window = ImGui::GetCurrentWindow();
    gizmoWindowFlags = ImGui::IsWindowHovered() &&
                               ImGui::IsMouseHoveringRect(window->InnerRect.Min,
                                                          window->InnerRect.Max)
                           ? ImGuiWindowFlags_NoMove
                           : 0;
  } else {
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
  }

  ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);
  ImGuizmo::DrawCubes(cameraView, cameraProjection, &objectMatrix[0][0],
                      gizmoCount);
  ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation,
                       mCurrentGizmoMode, matrix, NULL,
                       useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL,
                       boundSizingSnap ? boundsSnap : NULL);

  ImGuizmo::ViewManipulate(cameraView, camDistance,
                           ImVec2(viewManipulateRight - 128, viewManipulateTop),
                           ImVec2(128, 128), 0x10101010);

  if (useWindow) {
    ImGui::End();
    ImGui::PopStyleColor(1);
  }
}
