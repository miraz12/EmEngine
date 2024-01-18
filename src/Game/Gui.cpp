#include "Gui.hpp"

#include <SceneLoader.hpp>

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
GUI::GUI(Game &game) : m_game(game) {}

void GUI::renderGUI() {

  // Start the Dear ImGui frame

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin("Settings", 0, ImGuiWindowFlags_AlwaysAutoResize);

  if (ImGui::CollapsingHeader("Camera")) {
    float fov = m_game.m_ECSManager.getCamera().getFOV();
    ImGui::SliderFloat("FOV", &fov, 0.0f, 120.0f);
    m_game.m_ECSManager.getCamera().setFOV(fov);
    float nearFar[2];
    nearFar[0] = m_game.m_ECSManager.getCamera().getNear();
    nearFar[1] = m_game.m_ECSManager.getCamera().getFar();
    ImGui::InputFloat2("FOV", nearFar);
    m_game.m_ECSManager.getCamera().setNear(nearFar[0]);
    m_game.m_ECSManager.getCamera().setFar(nearFar[1]);
  }

  if (ImGui::CollapsingHeader("Lights")) {
    ImGui::SliderFloat3("Direction", glm::value_ptr(m_game.dirLightDir), -1.0f,
                        1.0f);
    ImGui::SliderFloat("Ambient", &m_game.dirLightAmbient, 0.0f, 2.0f);
    ImGui::ColorEdit3("Color", glm::value_ptr(m_game.dirLightColor));
  }

  if (ImGui::CollapsingHeader("Physics")) {
    ImGui::Checkbox("Enabled", &m_game.m_ECSManager.getSimulatePhysics());
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
    ImGui::Combo("views", &m_game.m_ECSManager.getDebugView(), &charitems[0], 7,
                 7);
  }

  Entity en = m_game.m_ECSManager.getPickedEntity();
  if (en > 0) {

    glm::vec3 &pos = m_game.m_ECSManager.getPosition(en);
    glm::quat &rot = m_game.m_ECSManager.getRotation(en);
    glm::vec3 &scale = m_game.m_ECSManager.getScale(en);

    ImGuizmo::BeginFrame();

    ImGui::Text("Selected entity: %lu", en);

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
    ImGui::InputFloat3("Tr", glm::value_ptr(pos));
    ImGui::InputFloat3("Rt", glm::value_ptr(rot));
    ImGui::InputFloat3("Sc", glm::value_ptr(scale));

    if (mCurrentGizmoOperation != ImGuizmo::SCALE) {
      if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
        mCurrentGizmoMode = ImGuizmo::LOCAL;
      ImGui::SameLine();
      if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
        mCurrentGizmoMode = ImGuizmo::WORLD;
    }
    API::Camera &cam = m_game.m_ECSManager.getCamera();
    glm::vec3 euler = glm::eulerAngles(rot) * RAD2DEG;
    editTransform(cam, pos, euler, scale);
    rot = glm::quat(euler * DEG2RAD);
  }
  if (ImGui::Button("Save scene")) {
    SceneLoader::getInstance().saveScene("resources/scene.yaml");
  }
  ImGui::End();
  // Rendering
  ImGui::Render();
}

void GUI::editTransform(API::Camera &camera, glm::vec3 &pos, glm::vec3 &rot,
                        glm::vec3 &scale) {

  glm::mat4 matrix = glm::identity<glm::mat4>();
  ImGuizmo::RecomposeMatrixFromComponents(
      glm::value_ptr(pos), glm::value_ptr(rot), glm::value_ptr(scale),
      glm::value_ptr(matrix));
  ImGuizmo::SetRect(0, 0, camera.getWidth(), camera.getHeight());
  ImGuizmo::SetOrthographic(false);
  ImGuizmo::Manipulate(glm::value_ptr(camera.getViewMatrix()),
                       glm::value_ptr(camera.getProjectionMatrix()),
                       mCurrentGizmoOperation, mCurrentGizmoMode,
                       glm::value_ptr(matrix), NULL, NULL);

  ImGuizmo::DecomposeMatrixToComponents(
      glm::value_ptr(matrix), glm::value_ptr(pos), glm::value_ptr(rot),
      glm::value_ptr(scale));

  // DEBUG grid
  // ImGuizmo::DrawGrid(glm::value_ptr(camera.getViewMatrix()),
  //                    glm::value_ptr(camera.getProjectionMatrix()),
  //                    glm::value_ptr(glm::identity<glm::mat4>()), 100.f);
}
