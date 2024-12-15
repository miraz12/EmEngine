#include "Gui.hpp"
#include "ECS/Components/PositionComponent.hpp"

#include <ECS/ECSManager.hpp>
#include <SceneLoader.hpp>

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

void
GUI::renderGUI()
{

  // Start the Dear ImGui frame

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::Begin("Settings", 0, ImGuiWindowFlags_AlwaysAutoResize);

  if (ImGui::CollapsingHeader("Camera")) {
    auto cam = static_pointer_cast<CameraComponent>(
      ECSManager::getInstance().getCamera());

    ImGui::SliderFloat("FOV", &cam->m_fov, 0.0f, 120.0f);
    float* nearFar[2];
    nearFar[0] = &cam->m_near;
    nearFar[1] = &cam->m_far;
    ImGui::InputFloat2("FOV", nearFar[0]);
  }

  // if (ImGui::CollapsingHeader("Lights")) {
  //   ImGui::SliderFloat3("Direction", glm::value_ptr(m_game.dirLightDir),
  //   -1.0f,
  //                       1.0f);
  //   ImGui::SliderFloat("Ambient", &m_game.dirLightAmbient, 0.0f, 2.0f);
  //   ImGui::ColorEdit3("Color", glm::value_ptr(m_game.dirLightColor));
  // }

  if (ImGui::CollapsingHeader("Physics")) {
    ImGui::Checkbox("Enabled", &ECSManager::getInstance().getSimulatePhysics());
  }

  static i32 offset = 0;
  offset = (offset + 1) % 50;
  if (ImGui::CollapsingHeader("Debug")) {
    // ImGui::PlotLines("FPS", fpsArray, 50, offset, nullptr, 0, 60,
    //                  ImVec2(0, 80.f));

    const std::vector<std::string> debugNamesInputs = {
      "none",     "Base color", "Normal",   "Occlusion",
      "Emissive", "Metallic",   "Roughness"
    };
    std::vector<const char*> charitems;
    charitems.reserve(debugNamesInputs.size());
    for (size_t i = 0; i < debugNamesInputs.size(); i++) {
      charitems.push_back(debugNamesInputs[i].c_str());
    }
    ImGui::Combo(
      "views", &ECSManager::getInstance().getDebugView(), &charitems[0], 7, 7);
  }

  Entity en = ECSManager::getInstance().getPickedEntity();
  if (en > 0) {

    glm::vec3& pos =
      ECSManager::getInstance().getComponent<PositionComponent>(en)->position;
    glm::quat& rot =
      ECSManager::getInstance().getComponent<PositionComponent>(en)->rotation;
    glm::vec3& scale =
      ECSManager::getInstance().getComponent<PositionComponent>(en)->scale;

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

    auto cam = static_pointer_cast<CameraComponent>(
      ECSManager::getInstance().getCamera());

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

void
GUI::editTransform(std::shared_ptr<CameraComponent> camera,
                   glm::vec3& pos,
                   glm::vec3& rot,
                   glm::vec3& scale)
{

  glm::mat4 matrix = glm::identity<glm::mat4>();
  ImGuizmo::RecomposeMatrixFromComponents(glm::value_ptr(pos),
                                          glm::value_ptr(rot),
                                          glm::value_ptr(scale),
                                          glm::value_ptr(matrix));
  ImGuizmo::SetRect(0, 0, camera->m_width, camera->m_height);
  ImGuizmo::SetOrthographic(false);
  ImGuizmo::Manipulate(glm::value_ptr(camera->m_viewMatrix),
                       glm::value_ptr(camera->m_ProjectionMatrix),
                       mCurrentGizmoOperation,
                       mCurrentGizmoMode,
                       glm::value_ptr(matrix),
                       NULL,
                       NULL);

  ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(matrix),
                                        glm::value_ptr(pos),
                                        glm::value_ptr(rot),
                                        glm::value_ptr(scale));

  // DEBUG grid
  // ImGuizmo::DrawGrid(glm::value_ptr(camera.getViewMatrix()),
  //                    glm::value_ptr(camera.getProjectionMatrix()),
  //                    glm::value_ptr(glm::identity<glm::mat4>()), 100.f);
}
