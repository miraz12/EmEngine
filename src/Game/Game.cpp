#include "Game.hpp"

#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/Components/LightingComponent.hpp>
#include <ECS/Components/PhysicsComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>
#include <Objects/Cube.hpp>
#include <Objects/GltfObject.hpp>
#include <Objects/Heightmap.hpp>
#include <SceneLoader.hpp>
#include <algorithm>

#include <glm/glm.hpp>
#include <memory>

#include "Window.hpp"
#include "glm/geometric.hpp"

Game::Game(GLFWwindow &window)
    : m_ECSManager(ECSManager::getInstance()), m_window(window),
      m_InputManager(InputManager::getInstance()) {
  SceneLoader::getInstance().init("resources/scene.yaml");
}

void Game::update(float dt) {
  m_ECSManager.updateDirLight(dirLightColor, dirLightAmbient, dirLightDir);
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
  float moveSpeed = 0.25f;
  Camera &cam = m_ECSManager.getCamera();
  // Parse input
  if (m_InputManager.keys.at(InputManager::KEY::A)) {
    if (debugMode) {
      btRigidBody *body = m_ECSManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(-1.0, 0.0, 0.0) + body->getLinearVelocity());
    } else {
      glm::vec3 camPos = cam.getPosition() -
                         glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed * dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
    }
  }
  if (m_InputManager.keys.at(InputManager::KEY::D)) {
    if (debugMode) {
      btRigidBody *body = m_ECSManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(1.0, 0.0, 0.0) + body->getLinearVelocity());
    } else {
      glm::vec3 camPos = cam.getPosition() +
                         glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed * dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
    }
  }
  if (m_InputManager.keys.at(InputManager::KEY::W)) {
    if (debugMode) {
      btRigidBody *body = m_ECSManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(0.0, 0.0, -1.0) + body->getLinearVelocity());
    } else {
      glm::vec3 camPos = cam.getPosition() + cam.getFront() * camSpeed * dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
    }
  }
  if (m_InputManager.keys.at(InputManager::KEY::S)) {
    if (debugMode) {
      btRigidBody *body = m_ECSManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(0.0, 0.0, 1.0) + body->getLinearVelocity());
    } else {
      glm::vec3 camPos = cam.getPosition() - cam.getFront() * camSpeed * dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
    }
  }
  if (m_InputManager.keys.at(InputManager::KEY::Space)) {
    if (debugMode) {
      btRigidBody *body = m_ECSManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(0.0, 1.0, 0.0) + body->getLinearVelocity());
    } else {
      glm::vec3 camPos = cam.getPosition() + glm::vec3(0, 1, 0) * camSpeed * dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
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
