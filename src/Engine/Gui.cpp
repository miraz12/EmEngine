#include "Gui.hpp"
#ifndef NDEBUG
#include "Profiler.hpp"
#include "ECS/Components/AnimationComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/DebugComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/ParticlesComponent.hpp"
#include "ECS/Components/PhysicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "Objects/Cube.hpp"
#include "Objects/GltfObject.hpp"
#include "Objects/Heightmap.hpp"
#include "Objects/Quad.hpp"
#include "ResourceManager.hpp"

#include <ECS/ECSManager.hpp>
#include <ECS/Systems/CameraSystem.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>
#include <SceneLoader.hpp>

#include <algorithm>
#include <filesystem>

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

void
GUI::renderGUI(Profiler& profiler)
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();

  drawSceneHierarchy();
  drawInspector();
  drawSettingsWindow(profiler);
  if (profiler.visible) {
    drawProfilerWindow(profiler);
  }
  drawCreateEntityPopup();

  ImGui::Render();
}

void
GUI::drawSceneHierarchy()
{
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
  ImGui::Begin("Scene Hierarchy");

  ImGui::InputTextWithHint("##filter", "Filter...", m_entityFilter, 128);
  ImGui::Separator();

  auto& ecsMan = ECSManager::getInstance();
  const auto& entities = ecsMan.getEntities();
  Entity picked = ecsMan.getPickedEntity();

  // Group entities by name prefix (strip trailing digits, underscores, hyphens)
  std::vector<std::pair<std::string, std::vector<Entity>>> groups;
  std::map<std::string, size_t> groupIndex;

  for (const Entity& en : entities) {
    std::string name(ecsMan.getEntityName(en));
    // Apply filter
    if (m_entityFilter[0] != '\0') {
      char label[256];
      snprintf(label, sizeof(label), "[%zu] %s", en, name.c_str());
      if (std::string_view(label).find(m_entityFilter) ==
          std::string_view::npos)
        continue;
    }
    // Strip trailing digits, underscores, hyphens to get group key
    std::string base = name;
    while (!base.empty() &&
           (std::isdigit(static_cast<unsigned char>(base.back())) ||
            base.back() == '_' || base.back() == '-'))
      base.pop_back();
    if (base.empty())
      base = name;

    auto it = groupIndex.find(base);
    if (it == groupIndex.end()) {
      groupIndex[base] = groups.size();
      groups.push_back({ base, { en } });
    } else {
      groups[it->second].second.push_back(en);
    }
  }

  bool entityDestroyed = false;
  auto drawEntity = [&](Entity en) {
    std::string_view name = ecsMan.getEntityName(en);
    char label[256];
    snprintf(
      label, sizeof(label), "[%zu] %.*s", en, (int)name.size(), name.data());
    bool isSelected = (en == picked);
    if (ImGui::Selectable(label, isSelected)) {
      ecsMan.setPickedEntity(en);
    }
    if (ImGui::BeginPopupContextItem()) {
      if (ImGui::MenuItem("Delete Entity")) {
        if (picked == en)
          ecsMan.setPickedEntity(0);
        ecsMan.destroyEntity(en);
        entityDestroyed = true;
        ImGui::EndPopup();
        return;
      }
      ImGui::EndPopup();
    }
  };

  ImGui::Text("Groups: %zu, Entities: %zu", groups.size(), entities.size());
  for (auto& [base, group] : groups) {
    if (entityDestroyed)
      break;
    if (group.size() == 1) {
      drawEntity(group[0]);
    } else {
      char groupLabel[256];
      snprintf(
        groupLabel, sizeof(groupLabel), "%s (%zu)", base.c_str(), group.size());
      if (ImGui::TreeNodeEx(groupLabel, ImGuiTreeNodeFlags_DefaultOpen)) {
        for (Entity en : group) {
          if (entityDestroyed)
            break;
          drawEntity(en);
        }
        ImGui::TreePop();
      }
    }
  }

  ImGui::Separator();
  if (ImGui::Button("Create Entity")) {
    m_showCreatePopup = true;
  }

  ImGui::End();
}

void
GUI::drawInspector()
{
  auto& ecsMan = ECSManager::getInstance();
  Entity en = ecsMan.getPickedEntity();

  ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 350, 0),
                          ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver);
  ImGui::Begin("Inspector");

  if (en == 0) {
    ImGui::TextDisabled("No entity selected");
    ImGui::End();
    return;
  }

  ImGui::Text("Entity: [%zu] %.*s",
              en,
              (int)ecsMan.getEntityName(en).size(),
              ecsMan.getEntityName(en).data());
  ImGui::Separator();

  // PositionComponent
  auto posComp = ecsMan.getComponent<PositionComponent>(en);
  if (posComp &&
      ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::InputFloat3("Position##pos", glm::value_ptr(posComp->position));
    glm::vec3 euler = glm::eulerAngles(posComp->rotation) * RAD2DEG;
    if (ImGui::InputFloat3("Rotation##pos", glm::value_ptr(euler))) {
      posComp->rotation = glm::quat(euler * DEG2RAD);
    }
    ImGui::InputFloat3("Scale##pos", glm::value_ptr(posComp->scale));

    // ImGuizmo gizmo
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

    if (mCurrentGizmoOperation != ImGuizmo::SCALE) {
      if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
        mCurrentGizmoMode = ImGuizmo::LOCAL;
      ImGui::SameLine();
      if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
        mCurrentGizmoMode = ImGuizmo::WORLD;
    }

    auto cam = CameraSystem::getInstance().getMainCameraComponent();
    if (cam) {
      glm::vec3 eulerDeg = glm::eulerAngles(posComp->rotation) * RAD2DEG;
      editTransform(cam, posComp->position, eulerDeg, posComp->scale);
      posComp->rotation = glm::quat(eulerDeg * DEG2RAD);

      // Sync transform to physics body when gizmo is manipulated
      if (ImGuizmo::IsUsing()) {
        auto phyComp = ecsMan.getComponent<PhysicsComponent>(en);
        if (phyComp && phyComp->isValid()) {
          auto& physSys = PhysicsSystem::getInstance();
          auto& bodyInterface =
            physSys.getBodyInterface();
          bodyInterface.SetPositionAndRotation(
            phyComp->getBodyID(),
            JPH::RVec3(
              posComp->position.x, posComp->position.y, posComp->position.z),
            JPH::Quat(posComp->rotation.x,
                       posComp->rotation.y,
                       posComp->rotation.z,
                       posComp->rotation.w),
            JPH::EActivation::Activate);
        }
      }
    }
  }

  // GraphicsComponent
  auto graComp = ecsMan.getComponent<GraphicsComponent>(en);
  if (graComp && ImGui::CollapsingHeader("Graphics")) {
    GraphicsObject* obj = graComp->m_grapObj.get();
    if (obj != nullptr) {
      ImGui::Text("Nodes: %u", obj->p_numNodes);
      ImGui::Text("Meshes: %u", obj->p_numMeshes);
      ImGui::Text("Materials: %u", obj->p_numMats);
      ImGui::Text("Animations: %u", obj->p_numAnimations);
    } else {
      ImGui::TextDisabled("No graphics object");
    }
  }

  // PhysicsComponent
  auto phyComp = ecsMan.getComponent<PhysicsComponent>(en);
  if (phyComp && ImGui::CollapsingHeader("Physics")) {
    ImGui::Text("Mass: %.2f", phyComp->getMass());
    const char* shapeNames[] = {
      "Box", "Sphere", "Capsule", "Convex Hull", "Heightmap"
    };
    ImGui::Text("Shape: %s",
                shapeNames[static_cast<int>(phyComp->getShapeType())]);
    ImGui::Text("Valid: %s", phyComp->isValid() ? "Yes" : "No");
  }

  // LightingComponent
  auto ligComp = ecsMan.getComponent<LightingComponent>(en);
  if (ligComp && ImGui::CollapsingHeader("Lighting")) {
    if (ligComp->getType() == LightingComponent::TYPE::DIRECTIONAL) {
      auto* dir = static_cast<DirectionalLight*>(&ligComp->getBaseLight());
      ImGui::Text("Type: Directional");
      ImGui::ColorEdit3("Color##lig", glm::value_ptr(dir->color));
      ImGui::InputFloat3("Direction##lig", glm::value_ptr(dir->direction));
      ImGui::InputFloat("Intensity##lig", &dir->intensity);
    } else if (ligComp->getType() == LightingComponent::TYPE::POINT) {
      auto* point = static_cast<PointLight*>(&ligComp->getBaseLight());
      ImGui::Text("Type: Point");
      ImGui::ColorEdit3("Color##lig", glm::value_ptr(point->color));
      ImGui::InputFloat3("Position##lig", glm::value_ptr(point->position));
      ImGui::InputFloat("Constant##lig", &point->constant);
      ImGui::InputFloat("Linear##lig", &point->linear);
      ImGui::InputFloat("Quadratic##lig", &point->quadratic);
    }
  }

  // ParticlesComponent
  auto parComp = ecsMan.getComponent<ParticlesComponent>(en);
  if (parComp && ImGui::CollapsingHeader("Particles")) {
    ImGui::InputFloat3("Velocity##par", glm::value_ptr(parComp->getVelocity()));
    ImGui::Text("Alive: %zu", parComp->getAliveParticles().size());
    ImGui::Text("Dead: %zu", parComp->getDeadParticles().size());
  }

  // CameraComponent
  auto camComp = ecsMan.getComponent<CameraComponent>(en);
  if (camComp && ImGui::CollapsingHeader("Camera")) {
    ImGui::SliderFloat("FOV##cam", &camComp->m_fov, 1.0f, 120.0f);
    ImGui::InputFloat("Near##cam", &camComp->m_near);
    ImGui::InputFloat("Far##cam", &camComp->m_far);
    bool isMain = (CameraSystem::getInstance().getMainCamera() == en);
    if (isMain) {
      ImGui::TextDisabled("Main Camera");
    } else {
      if (ImGui::Button("Set as Main Camera##cam")) {
        CameraSystem::getInstance().setMainCamera(en);
      }
    }
    ImGui::Text("Position: (%.1f, %.1f, %.1f)",
                camComp->m_position.x,
                camComp->m_position.y,
                camComp->m_position.z);
    ImGui::Text("Front: (%.2f, %.2f, %.2f)",
                camComp->m_front.x,
                camComp->m_front.y,
                camComp->m_front.z);
  }

  // AnimationComponent
  auto animComp = ecsMan.getComponent<AnimationComponent>(en);
  if (animComp && ImGui::CollapsingHeader("Animation")) {
    ImGui::Checkbox("Playing##anim", &animComp->isPlaying);
    int idx = static_cast<int>(animComp->animationIndex);
    if (ImGui::InputInt("Animation##anim", &idx)) {
      if (idx >= 0)
        animComp->animationIndex = static_cast<u32>(idx);
    }
    ImGui::Text("Time: %.2f", animComp->currentTime);
    if (animComp->blending) {
      ImGui::Text("Blending: %.0f%% (%.1fs / %.1fs)",
                  animComp->blendWeight * 100.0f,
                  animComp->blendElapsed,
                  animComp->blendDuration);
    }
  }

  // DebugComponent
  auto debComp = ecsMan.getComponent<DebugComponent>(en);
  if (debComp && ImGui::CollapsingHeader("Debug")) {
    ImGui::TextDisabled("Debug visualization attached");
  }

  // Add Component section
  ImGui::Separator();
  if (ImGui::CollapsingHeader("Add Component")) {
    if (!posComp && ImGui::Button("+ Position")) {
      ecsMan.addComponents(en, std::make_shared<PositionComponent>());
    }
    if (!camComp && ImGui::Button("+ Camera")) {
      ecsMan.addComponents(
        en,
        std::make_shared<CameraComponent>(45.0f, 800.0f, 600.0f, 0.1f, 200.0f));
    }
    if (!animComp && graComp &&
        graComp->type == GraphicsComponent::TYPE::MESH &&
        graComp->m_grapObj->p_numAnimations > 0 &&
        ImGui::Button("+ Animation")) {
      ecsMan.addComponents(en, std::make_shared<AnimationComponent>());
    }
  }

  ImGui::End();
}

void
GUI::drawSettingsWindow(Profiler& profiler)
{
  ImGui::Begin("Settings", 0, ImGuiWindowFlags_AlwaysAutoResize);

  if (ImGui::CollapsingHeader("Physics")) {
    ImGui::Checkbox("Enabled", &ECSManager::getInstance().refSimulatePhysics());
  }

  if (ImGui::CollapsingHeader("Debug")) {
    const std::vector<std::string> debugNamesInputs = {
      "none",     "Base color", "Normal",    "Occlusion",
      "Emissive", "Metallic",   "Roughness", "Shadows"
    };
    std::vector<const char*> charitems;
    charitems.reserve(debugNamesInputs.size());
    for (size_t i = 0; i < debugNamesInputs.size(); i++) {
      charitems.push_back(debugNamesInputs[i].c_str());
    }
    ImGui::Combo("views",
                 &ECSManager::getInstance().refDebugView(),
                 &charitems[0],
                 debugNamesInputs.size(),
                 debugNamesInputs.size());
  }

  if (ImGui::Button("Save scene")) {
    SceneLoader::getInstance().saveScene("resources/scene.yaml");
  }

  ImGui::Separator();
  ImGui::Checkbox("Show Profiler", &profiler.visible);

  ImGui::End();
}

void
GUI::drawProfilerWindow(Profiler& profiler)
{
  ImGui::SetNextWindowPos(ImVec2(0, 420), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_FirstUseEver);
  ImGui::Begin("Performance Profiler", &profiler.visible);

  // FPS counter
  ImGui::Text(
    "FPS: %.1f  |  Frame: %.2f ms", profiler.getFps(), profiler.getFrameTimeMs());

  // FPS history graph
  auto& fpsHist = profiler.fpsHistory();
  if (fpsHist.count > 0) {
    char overlay[32];
    snprintf(overlay, sizeof(overlay), "%.1f FPS", profiler.getFps());
    ImGui::PlotLines(
      "##fps",
      RingBuffer::getter,
      &fpsHist,
      static_cast<int>(fpsHist.count),
      0,
      overlay,
      0.0f,
      200.0f,
      ImVec2(0, 60));
  }

  // Frame time history graph
  auto& ftHist = profiler.frameTimeHistory();
  if (ftHist.count > 0) {
    char overlay[32];
    snprintf(overlay, sizeof(overlay), "%.2f ms", profiler.getFrameTimeMs());
    ImGui::PlotLines(
      "##frametime",
      RingBuffer::getter,
      &ftHist,
      static_cast<int>(ftHist.count),
      0,
      overlay,
      0.0f,
      33.3f,
      ImVec2(0, 60));
  }

  ImGui::Separator();

  // Per-system CPU timing
  if (ImGui::CollapsingHeader("ECS Systems", ImGuiTreeNodeFlags_DefaultOpen)) {
    float totalSys = 0.0f;
    float maxSys = 0.0f;
    for (size_t idx = 0; idx < profiler.getSectionCount(); ++idx) {
      if (profiler.getSection(idx).category == SectionCategory::kSystem) {
        totalSys += profiler.getSection(idx).durationMs;
        maxSys = std::max(maxSys, profiler.getSection(idx).durationMs);
      }
    }
    for (size_t idx = 0; idx < profiler.getSectionCount(); ++idx) {
      const auto& sec = profiler.getSection(idx);
      if (sec.category != SectionCategory::kSystem)
        continue;
      float frac = (maxSys > 0.001f) ? sec.durationMs / maxSys : 0.0f;
      char label[64];
      snprintf(label,
               sizeof(label),
               "%.*s: %.3f ms",
               static_cast<int>(sec.name.size()),
               sec.name.data(),
               sec.durationMs);
      ImGui::ProgressBar(frac, ImVec2(-1, 0), label);
    }
    ImGui::Text("Total ECS: %.3f ms", totalSys);
  }

  // Per-render-pass CPU timing
  if (ImGui::CollapsingHeader("Render Passes", ImGuiTreeNodeFlags_DefaultOpen)) {
    float totalPass = 0.0f;
    float maxPass = 0.0f;
    for (size_t idx = 0; idx < profiler.getSectionCount(); ++idx) {
      if (profiler.getSection(idx).category == SectionCategory::kRenderPass) {
        totalPass += profiler.getSection(idx).durationMs;
        maxPass = std::max(maxPass, profiler.getSection(idx).durationMs);
      }
    }
    for (size_t idx = 0; idx < profiler.getSectionCount(); ++idx) {
      const auto& sec = profiler.getSection(idx);
      if (sec.category != SectionCategory::kRenderPass)
        continue;
      float frac = (maxPass > 0.001f) ? sec.durationMs / maxPass : 0.0f;
      char label[64];
      snprintf(label,
               sizeof(label),
               "%.*s: %.3f ms",
               static_cast<int>(sec.name.size()),
               sec.name.data(),
               sec.durationMs);
      ImGui::ProgressBar(frac, ImVec2(-1, 0), label);
    }
    ImGui::Text("Total Render: %.3f ms", totalPass);
  }

  // Frame breakdown
  if (ImGui::CollapsingHeader("Frame Breakdown", ImGuiTreeNodeFlags_DefaultOpen)) {
    static constexpr const char* phaseNames[] = { "ECS Update", "UI", "Swap" };
    for (size_t idx = 0; idx < Profiler::kNumPhases; ++idx) {
      ImGui::Text("%-12s: %.3f ms", phaseNames[idx], profiler.getPhaseMs(idx));
    }
    ImGui::Text("Total Frame:  %.3f ms", profiler.getFrameTimeMs());
  }

  ImGui::End();
}

void
GUI::drawCreateEntityPopup()
{
  if (m_showCreatePopup) {
    ImGui::OpenPopup("Create Entity");
    m_showCreatePopup = false;
  }

  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

  if (ImGui::BeginPopupModal(
        "Create Entity", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::InputText("Name", m_newEntityName, 128);

    ImGui::Separator();
    ImGui::Text("Components:");
    ImGui::Checkbox("Position", &m_createWithPosition);
    ImGui::Checkbox("Graphics", &m_createWithGraphics);
    ImGui::Checkbox("Physics", &m_createWithPhysics);

    if (m_createWithGraphics) {
      const char* primitiveTypes[] = { "Quad", "Cube", "Mesh", "Heightmap" };
      ImGui::Combo("Primitive", &m_createGraphicsType, primitiveTypes, 4);

      if (m_createGraphicsType == 2) { // Mesh
        if (!m_modelsScanned) {
          scanAvailableModels();
        }
        if (m_availableModels.empty()) {
          ImGui::TextDisabled("No models found in resources/Models/gltf/");
        } else {
          const char* preview = m_selectedModel >= 0
                                  ? m_availableModels[m_selectedModel].c_str()
                                  : "Select a model...";
          if (ImGui::BeginCombo("Model", preview)) {
            for (int i = 0; i < static_cast<int>(m_availableModels.size());
                 i++) {
              bool isSelected = (m_selectedModel == i);
              if (ImGui::Selectable(m_availableModels[i].c_str(), isSelected)) {
                m_selectedModel = i;
              }
              if (isSelected) {
                ImGui::SetItemDefaultFocus();
              }
            }
            ImGui::EndCombo();
          }
          if (ImGui::Button("Refresh Models")) {
            m_modelsScanned = false;
          }
        }
      } else if (m_createGraphicsType == 3) { // Heightmap
        ImGui::InputTextWithHint(
          "Heightmap Path", "heightmap.png", m_createHeightmapPath, 256);
      }
    }

    if (m_createWithPhysics) {
      ImGui::InputFloat("Mass", &m_createMass);
      const char* shapeTypes[] = {
        "Box", "Sphere", "Capsule", "Convex Hull", "Heightmap"
      };
      ImGui::Combo("Shape", &m_createPhysicsShape, shapeTypes, 5);
    }

    ImGui::Separator();
    if (ImGui::Button("Create", ImVec2(120, 0))) {
      auto& ecsMan = ECSManager::getInstance();
      auto& resMgr = ResourceManager::getInstance();

      Entity newEntity = ecsMan.createEntity(m_newEntityName);

      if (m_createWithPosition) {
        ecsMan.addComponents(newEntity, std::make_shared<PositionComponent>());
      }

      if (m_createWithGraphics) {
        std::shared_ptr<GraphicsComponent> graphComp;
        switch (m_createGraphicsType) {
          case 0: // Quad
            graphComp = std::make_shared<GraphicsComponent>(
              std::static_pointer_cast<GraphicsObject>(resMgr.getQuad()));
            graphComp->type = GraphicsComponent::TYPE::QUAD;
            break;
          case 1: // Cube
            graphComp = std::make_shared<GraphicsComponent>(
              std::static_pointer_cast<GraphicsObject>(resMgr.getCube()));
            graphComp->type = GraphicsComponent::TYPE::CUBE;
            break;
          case 2: // Mesh
            if (m_selectedModel >= 0) {
              std::string path =
                "resources/Models/" + m_availableModels[m_selectedModel];
              graphComp = std::make_shared<GraphicsComponent>(
                std::static_pointer_cast<GraphicsObject>(
                  resMgr.getGltfModel(path)));
              graphComp->type = GraphicsComponent::TYPE::MESH;
              if (graphComp->m_grapObj->p_numAnimations > 0) {
                ecsMan.addComponents(newEntity,
                                     std::make_shared<AnimationComponent>());
              }
            }
            break;
          case 3: // Heightmap
            if (m_createHeightmapPath[0] != '\0') {
              std::string path =
                "resources/Textures/" + std::string(m_createHeightmapPath);
              graphComp = std::make_shared<GraphicsComponent>(
                std::static_pointer_cast<GraphicsObject>(
                  resMgr.getHeightmap(path)));
              graphComp->type = GraphicsComponent::TYPE::HEIGHTMAP;
            }
            break;
        }
        if (graphComp) {
          ecsMan.addComponents(newEntity, graphComp);
        }
      }

      if (m_createWithPhysics) {
        ecsMan.addComponents(
          newEntity,
          std::make_shared<PhysicsComponent>(
            newEntity, m_createMass, CollisionShapeType(m_createPhysicsShape)));
      }

      ecsMan.setPickedEntity(newEntity);
      ImGui::CloseCurrentPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void
GUI::scanAvailableModels()
{
  m_availableModels.clear();
  m_selectedModel = -1;

  const std::filesystem::path modelsDir("resources/Models");
  if (!std::filesystem::exists(modelsDir)) {
    m_modelsScanned = true;
    return;
  }

  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(modelsDir)) {
    if (!entry.is_regular_file())
      continue;
    auto ext = entry.path().extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".glb" || ext == ".gltf") {
      // Store path relative to resources/Models/
      auto relPath =
        std::filesystem::relative(entry.path(), modelsDir).string();
      m_availableModels.push_back(relPath);
    }
  }

  std::sort(m_availableModels.begin(), m_availableModels.end());
  m_modelsScanned = true;
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
}

#endif // !NDEBUG
