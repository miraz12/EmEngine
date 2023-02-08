#ifndef ECSMANAGER_H_
#define ECSMANAGER_H_

#include <GLFW/glfw3.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "Components/GraphicsComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Entity.hpp"
#include "Objects/Mesh.hpp"
#include "Objects/Quad.hpp"
#include "ShaderPrograms/MeshShaderProgram.hpp"
#include "ShaderPrograms/SimpleShaderProgram.hpp"
#include "Systems/LightingSystem.hpp"

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
  Entity &createEntity();

  // adds entity and inserts it into to all matching systems
  void addEntity(Entity *entity);

  // adds component to entity and inserts it to matching systems
  void addComponent(Entity &entity, Component *component);

  // Removes an entity from all systems
  void removeEntity(int entityID);

  // Removes  componenet from entity and then the entity from all concerned
  // systems
  void removeComponent(Entity &entity, ComponentTypeEnum component);

  // Create a controllable entity at position x,y
  int createPlayerEntity(float x, float y, GLFWwindow *window);

  // Create point light
  void SetupPointLight(glm::vec3 color, float constant, float linear, float quadratic,
                       glm::vec3 pos);

  // Create directional light
  void SetupDirectionalLight(glm::vec3 color, float ambient, glm::vec3 dir);

  // Returns entity by ID, or NULL if it does not exist
  static Entity *getEntity(int entityID);

  // Get rendering system
  LightingSystem *getLightingSystem() {
    return dynamic_cast<LightingSystem *>(m_systems["LIGHTS"]);
  };

  Camera &getCamera() { return m_camera; };

private:
  ECSManager();
  // Entities
  int m_idCounter;
  static std::vector<Entity *> m_entities;

  // Systems
  std::map<std::string, System *> m_systems;

  // Events
  std::vector<Entity *> m_addEntities;
  struct addComponent_t {
    Entity &ent;
    Component *cmp;
  };
  std::vector<addComponent_t> m_addComponents;
  std::vector<int> m_removeEntities;
  struct removeComponent_t {
    Entity &ent;
    ComponentTypeEnum cmp;
  };
  std::vector<removeComponent_t> m_removeComponents;
  Camera m_camera;

  void addEntities();
  void addComponents();
  void removeEntities();
  void removeComponents();
};
#endif // LIGHTINGSYSTEM_H_
