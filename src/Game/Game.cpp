#include "Game.hpp"

#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/Components/LightingComponent.hpp>
#include <ECS/Components/ParticlesComponent.hpp>
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
  Entity e = m_ECSManager.createEntity();
  std::shared_ptr<ParticlesComponent> parComp = std::make_shared<ParticlesComponent>(
      glm::vec3(0.0f, -0.25f, -1.0f), glm::vec3(0.0f, 5.0f, 0.0f));
  m_ECSManager.addComponent(e, parComp);
}

void Game::update(float dt) {
  m_InputManager.update(dt);
  m_ECSManager.updateDirLight(dirLightColor, dirLightAmbient, dirLightDir);
  m_ECSManager.dDir = dirLightDir;
  m_ECSManager.update(dt);
}

void Game::setViewport(uint32_t w, uint32_t h) { m_ECSManager.setViewport(w, h); }

Game::~Game() {}
