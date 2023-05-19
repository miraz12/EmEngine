#include "Game.hpp"

#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/Components/LightingComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <Objects/GltfObject.hpp>
#include <algorithm>

#include <glm/glm.hpp>

#include "Window.hpp"
#include "glm/geometric.hpp"

Game::Game(GLFWwindow *window)
    : m_window(window), m_ECSManager(&ECSManager::getInstance()),
      m_InputManager(&InputManager::getInstance()) {

  // TODO dynamic shadow projection and view
  // GltfObject *g = new GltfObject("../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf");
  // GltfObject *g =
  // new GltfObject("../glTF-Sample-Models/2.0/ABeautifulGame/glTF/ABeautifulGame.gltf");

  // ----
  Entity en = m_ECSManager->createEntity();
  GraphicsComponent *graphComp = new GraphicsComponent(*new GltfObject("resources/Models/gltf/helmet/DamagedHelmet.gltf"));
  // GraphicsComponent *graphComp = new GraphicsComponent(*new GltfObject("../glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf"));
  PositionComponent *posComp = new PositionComponent();
  m_ECSManager->addComponents<GraphicsComponent, PositionComponent>(en, graphComp, posComp);

  // Entity en = m_ECSManager->createEntity();
  // GraphicsComponent *graphComp = new GraphicsComponent();
  // graphComp->grapObj = m;
  // PositionComponent *posComp = new PositionComponent();
  // m_ECSManager->addComponents<GraphicsComponent, PositionComponent>(en, graphComp, posComp);

  // ----
  dirLightColor = glm::vec3(0.988f, 0.898f, 0.439f);
  dirLightAmbient = 0.6f;
  dirLightDir = glm::vec3(0.01f, -1.0f, 0.0f);
  dLight = m_ECSManager->SetupDirectionalLight(dirLightColor, dirLightAmbient, dirLightDir);

  m_ECSManager->SetupPointLight(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 0.7f, 1.8f,
                                glm::vec3(0.0f, 1.0f, 0.0f));

  m_ECSManager->SetupPointLight(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.7f, 1.8f,
                                glm::vec3(1.5f, 1.0f, 0.0f));

  m_ECSManager->SetupPointLight(glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.7f, 1.8f,
                                glm::vec3(-1.5f, 1.0f, 0.0f));
}

void Game::update(float dt) {
  dLight->color = dirLightColor;
  dLight->ambientIntensity = dirLightAmbient;
  dLight->direction = dirLightDir;
  handleInput(dt);
  m_ECSManager->update(dt);
  m_ECSManager->dDir = dirLightDir;
}

void Game::handleInput(float dt) {
  float camSpeed = 1.f;
  Camera &cam = m_ECSManager->getCamera();
  // Parse input
  if (m_InputManager->keys.at(InputManager::KEY::A)) {
    glm::vec3 camPos =
        cam.getPosition() - glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed * dt;
    if (!glm::all(glm::isnan(camPos))) {
      cam.setPosition(camPos);
    }
  }
  if (m_InputManager->keys.at(InputManager::KEY::D)) {
    glm::vec3 camPos =
        cam.getPosition() + glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed * dt;
    if (!glm::all(glm::isnan(camPos))) {
      cam.setPosition(camPos);
    }
  }
  if (m_InputManager->keys.at(InputManager::KEY::W)) {
    glm::vec3 camPos = cam.getPosition() + cam.getFront() * camSpeed * dt;
    if (!glm::all(glm::isnan(camPos))) {
      cam.setPosition(camPos);
    }
  }
  if (m_InputManager->keys.at(InputManager::KEY::S)) {
    glm::vec3 camPos = cam.getPosition() - cam.getFront() * camSpeed * dt;
    if (!glm::all(glm::isnan(camPos))) {
      cam.setPosition(camPos);
    }
  }
  if (m_InputManager->keys.at(InputManager::KEY::Mouse1)) {
    glm::vec3 direction;
    direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    direction.y = sin(glm::radians(m_pitch));
    direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    glm::vec3 cameraFront = glm::normalize(direction);
    cam.setFront(cameraFront);
  }
}

void Game::setViewport(unsigned int w, unsigned int h) { m_ECSManager->setViewport(w, h); }

Game::~Game() {}
