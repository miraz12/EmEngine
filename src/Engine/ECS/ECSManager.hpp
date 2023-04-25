#ifndef ECSMANAGER_H_
#define ECSMANAGER_H_

#include <GLFW/glfw3.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "Components/GraphicsComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "Objects/Mesh.hpp"
#include "Objects/Quad.hpp"
#include "ShaderPrograms/MeshShaderProgram.hpp"
#include "ShaderPrograms/SimpleShaderProgram.hpp"
#include "Systems/LightingSystem.hpp"

using Entity = std::size_t;

class ECSManager {
public:
  static ECSManager &getInstance() {
    static ECSManager instance;
    return instance;
  }

  ECSManager(ECSManager const &) = delete;
  void operator=(ECSManager const &) = delete;

  ~ECSManager();

  void initializeSystems();

  // Runs through all systems
  void update(float dt);

  // resets ECS
  void reset();

  void updateRenderingSystems(float dt);

  // creates and returns a new entity
  Entity createEntity();

  // adds entity and inserts it into to all matching systems
  void addEntity(Entity *entity);

  // adds component to entity and inserts it to matching systems
  template <typename T> void addComponent(Entity entity, T component);

  // Removes an entity from all systems
  template <typename T>
  void removeEntity(int entityID);

  // Removes  componenet from entity and then the entity from all concerned
  // systems
  template <typename T>
  void removeComponent(Entity &entity, T component);

  // Create a controllable entity at position x,y
  int createPlayerEntity(float x, float y, GLFWwindow *window);

  // Create point light
  PointLight *SetupPointLight(glm::vec3 color, float constant, float linear, float quadratic,
                              glm::vec3 pos);

  // Create directional light
  DirectionalLight *SetupDirectionalLight(glm::vec3 color, float ambient, glm::vec3 dir);

  // Returns entity by ID, or NULL if it does not exist
  Entity *getEntity(int entityID);

  Camera &getCamera() { return m_camera; };

private:
  ECSManager();
  // Entities
  int m_idCounter;
  std::vector<Entity> m_entities;
  std::unordered_map<Entity, ComponentType> m_components;

  size_t m_entityCount = 0;  // Number of entities created so far

  Camera m_camera;
};
#endif // LIGHTINGSYSTEM_H_
