#include "Game.hpp"

#include <ECS/Components/LightingComponent.hpp>
#include <algorithm>
#include <glm/glm.hpp>

#include "Window.hpp"
#include "glm/geometric.hpp"

Game::Game(GLFWwindow *window)
    : m_window(window), m_ECSManager(&ECSManager::getInstance()),
      m_InputManager(&InputManager::getInstance()) {
  m_ECSManager->createPlayerEntity(0, 0, m_window);

  Entity &en = m_ECSManager->createEntity();
  en.setName("Quad");
  GraphicsComponent *graphComp = new GraphicsComponent();
  SimpleShaderProgram *p = new SimpleShaderProgram;
  graphComp->grapObj = new Quad(*p);
  m_ECSManager->addComponent(en, graphComp);
  PositionComponent *posComp = new PositionComponent();
  posComp->position = glm::vec3(0.0, 0.0, 2.0);
  m_ECSManager->addComponent(en, posComp);

  Entity &en2 = m_ECSManager->createEntity();
  LightingComponent *lightComp = new LightingComponent();
  lightComp->SetupDirectionalLight(glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, 0.01f,
                                   glm::vec3(1.0f, -1.0f, 0.0f));
  m_ECSManager->addComponent(en2, lightComp);

  Entity &en3 = m_ECSManager->createEntity();
  LightingComponent *lightComp2 = new LightingComponent();
  lightComp2->SetupPointLight(glm::vec3(1.0f, 0.5f, 0.0f), 0.1f, 0.1f, 0.0f, 0.1f, 0.0f,
                              glm::vec3(3.0f, 1.0f, 0.0f));
  m_ECSManager->addComponent(en3, lightComp2);
}

void Game::update(float dt) {
  handleInput(dt);
  m_ECSManager->update(dt);
}

void Game::handleInput(float dt) {
  float camSpeed = 5.f;
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

void Game::setViewport(unsigned int w, unsigned int h) {
  m_ECSManager->getLightingSystem()->setViewport(w, h);
}

Game::~Game() {}
