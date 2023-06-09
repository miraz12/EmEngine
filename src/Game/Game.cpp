#include "Game.hpp"

#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/Components/LightingComponent.hpp>
#include <ECS/Components/PhysicsComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>
#include <Objects/Cube.hpp>
#include <Objects/GltfObject.hpp>
#include <algorithm>

#include <glm/glm.hpp>
#include <memory>

#include "Window.hpp"
#include "glm/geometric.hpp"

Game::Game(GLFWwindow &window)
    : m_ECSManager(ECSManager::getInstance()), m_window(window),
      m_InputManager(InputManager::getInstance()) {

  // TODO dynamic shadow projection and view
  // ----
  Entity en = m_ECSManager.createEntity();
  std::shared_ptr<GraphicsComponent> graphComp = std::make_shared<GraphicsComponent>(
      *new GltfObject("resources/Models/gltf/helmet/DamagedHelmet.gltf"));
  // *new GltfObject("../glTF-Sample-Models/2.0/Suzanne/glTF/Suzanne.gltf"));
  // *new Cube());
  // *new
  // GltfObject("../glTF-Sample-Models/2.0/MetalRoughSpheres/glTF/MetalRoughSpheres.gltf"));
  // *new GltfObject("../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf"));
  std::shared_ptr<PositionComponent> posComp = std::make_shared<PositionComponent>();
  posComp->position = glm::vec3(0.0, 2.0, -1.0);
  std::shared_ptr<PhysicsComponent> physComp = std::make_shared<PhysicsComponent>(posComp, 1.0f);
  m_ECSManager.addComponents<GraphicsComponent, PositionComponent, PhysicsComponent>(
      en, graphComp, posComp, physComp);
  // ----
  Entity en2 = m_ECSManager.createEntity();
  graphComp = std::make_shared<GraphicsComponent>(*new Cube());
  posComp = std::make_shared<PositionComponent>();
  posComp->scale = glm::vec3(20.0, 1.0, 20.0);
  posComp->position = glm::vec3(0.0, -2.0, 0.0);
  physComp = std::make_shared<PhysicsComponent>(posComp);
  m_ECSManager.addComponents<GraphicsComponent, PositionComponent, PhysicsComponent>(
      en2, graphComp, posComp, physComp);

  // ----
  dirLightColor = glm::vec3(0.988f, 0.898f, 0.439f);
  dirLightAmbient = 0.1f;
  dirLightDir = glm::vec3(0.01f, -1.0f, 0.0f);
  dLight = m_ECSManager.SetupDirectionalLight(dirLightColor, dirLightAmbient, dirLightDir);

  m_ECSManager.SetupPointLight(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.7f, 1.8f,
                               glm::vec3(0.0f, 1.0f, 0.0f));

  m_ECSManager.SetupPointLight(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.7f, 1.8f,
                               glm::vec3(1.5f, 1.0f, 0.0f));

  m_ECSManager.SetupPointLight(glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.7f, 1.8f,
                               glm::vec3(-1.5f, 1.0f, 0.0f));
}

void Game::update(float dt) {
  dLight->color = dirLightColor;
  dLight->ambientIntensity = dirLightAmbient;
  dLight->direction = dirLightDir;
  handleInput(dt);
  m_ECSManager.update(dt);
  if (m_ECSManager.dDir != dirLightDir) {
    m_ECSManager.dDir = dirLightDir;
    m_ECSManager.dirDirty = true;
  }
  m_ECSManager.debugView = debugView;
  m_ECSManager.debugMode = debugMode;
}

void Game::handleInput(float dt) {
  float camSpeed = 1.f;
  Camera &cam = m_ECSManager.getCamera();
  // Parse input
  if (m_InputManager.keys.at(InputManager::KEY::A)) {
    glm::vec3 camPos =
        cam.getPosition() - glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed * dt;
    if (!glm::all(glm::isnan(camPos))) {
      cam.setPosition(camPos);
    }
  }
  if (m_InputManager.keys.at(InputManager::KEY::D)) {
    glm::vec3 camPos =
        cam.getPosition() + glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed * dt;
    if (!glm::all(glm::isnan(camPos))) {
      cam.setPosition(camPos);
    }
  }
  if (m_InputManager.keys.at(InputManager::KEY::W)) {
    glm::vec3 camPos = cam.getPosition() + cam.getFront() * camSpeed * dt;
    if (!glm::all(glm::isnan(camPos))) {
      cam.setPosition(camPos);
    }
  }
  if (m_InputManager.keys.at(InputManager::KEY::S)) {
    glm::vec3 camPos = cam.getPosition() - cam.getFront() * camSpeed * dt;
    if (!glm::all(glm::isnan(camPos))) {
      cam.setPosition(camPos);
    }
  }
  if (m_InputManager.keys.at(InputManager::KEY::O)) {
    m_ECSManager.debugMode = m_ECSManager.debugMode ? false : true;
  }

  static bool pressed = true;
  if (m_InputManager.keys.at(InputManager::KEY::Mouse1)) {
    glm::vec3 direction;
    direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    direction.y = sin(glm::radians(m_pitch));
    direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    glm::vec3 cameraFront = glm::normalize(direction);
    cam.setFront(cameraFront);
    if (pressed) {
      PhysicsSystem::getInstance().performPicking(m_mousePosX, m_mousePosY);
      pressed = false;
    }
  } else {
    pressed = true;
  }
}

void Game::setViewport(uint32_t w, uint32_t h) { m_ECSManager.setViewport(w, h); }

Game::~Game() {}
