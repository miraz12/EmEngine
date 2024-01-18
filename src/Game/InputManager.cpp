#include "InputManager.hpp"
#include "ECS/ECSManager.hpp"
#include <Camera.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>

InputManager::InputManager() {
  keys.insert({KEY::Escape, false});
  keys.insert({KEY::W, false});
  keys.insert({KEY::A, false});
  keys.insert({KEY::S, false});
  keys.insert({KEY::D, false});
  keys.insert({KEY::F, false});
  keys.insert({KEY::O, false});
  keys.insert({KEY::Space, false});
  keys.insert({KEY::ArrowUp, false});
  keys.insert({KEY::ArrowDown, false});
  keys.insert({KEY::ArrowRight, false});
  keys.insert({KEY::ArrowLeft, false});
  keys.insert({KEY::Mouse1, false});
}

void InputManager::handleInput(KEY k, bool pressed) { keys.at(k) = pressed; }

void InputManager::update(float dt) {
  float camSpeed = 1.f;
  float moveSpeed = 0.25f;
  ECSManager &ecsManager = ECSManager::getInstance();
  Camera &cam = ecsManager.getCamera();
  // Parse input
  if (keys.at(InputManager::KEY::A)) {
    if (ecsManager.getSimulatePhysics()) {
      btRigidBody *body =
          ecsManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(-1.0, 0.0, 0.0) +
                              body->getLinearVelocity());
    } else {
      glm::vec3 camPos =
          cam.getPosition() -
          glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed *
              dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
    }
  }
  if (keys.at(InputManager::KEY::D)) {
    if (ecsManager.getSimulatePhysics()) {
      btRigidBody *body =
          ecsManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(1.0, 0.0, 0.0) +
                              body->getLinearVelocity());
    } else {
      glm::vec3 camPos =
          cam.getPosition() +
          glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed *
              dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
    }
  }
  if (keys.at(InputManager::KEY::W)) {
    if (ecsManager.getSimulatePhysics()) {
      btRigidBody *body =
          ecsManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(0.0, 0.0, -1.0) +
                              body->getLinearVelocity());
    } else {
      glm::vec3 camPos = cam.getPosition() + cam.getFront() * camSpeed * dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
    }
  }
  if (keys.at(InputManager::KEY::S)) {
    if (ecsManager.getSimulatePhysics()) {
      btRigidBody *body =
          ecsManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(0.0, 0.0, 1.0) +
                              body->getLinearVelocity());
    } else {
      glm::vec3 camPos = cam.getPosition() - cam.getFront() * camSpeed * dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
    }
  }
  if (keys.at(InputManager::KEY::Space)) {
    if (ecsManager.getSimulatePhysics()) {
      btRigidBody *body =
          ecsManager.getComponent<PhysicsComponent>(m_player)->getRigidBody();
      body->activate();
      body->setLinearVelocity(moveSpeed * btVector3(0.0, 1.0, 0.0) +
                              body->getLinearVelocity());
    } else {
      glm::vec3 camPos = cam.getPosition() + glm::vec3(0, 1, 0) * camSpeed * dt;
      if (!glm::all(glm::isnan(camPos))) {
        cam.setPosition(camPos);
      }
    }
  }
  if (keys.at(InputManager::KEY::O)) {
    ecsManager.setSimulatePhysics(ecsManager.getSimulatePhysics() ? false
                                                                  : true);
  }

  static bool pressed = true;
  if (keys.at(InputManager::KEY::Mouse1)) {
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
