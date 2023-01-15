#include "ECSManager.hpp"

std::vector<Entity *> ECSManager::m_entities;

ECSManager::ECSManager()
    : m_idCounter(1), m_addEntities(), m_addComponents(), m_removeEntities(),
      m_removeComponents() {
  initializeSystems();
}

ECSManager::~ECSManager() {
  // Delete all entities and systems
  for (auto &e : m_entities) {
    delete e;
  }
  for (auto &s : m_systems) {
    delete s.second;
  }
}

void ECSManager::initializeSystems() {
  m_systems["GRAPHICS"] = new GraphicsSystem(this, m_camera);
}

void ECSManager::update(float dt) {
  // for all entities, remove/add components
  // remove/add entities from systems
  addEntities();
  addComponents();
  removeEntities();
  removeComponents();

  // update all systems
  for (auto &s : m_systems) {
    s.second->update(dt);
  }
}

void ECSManager::reset() {
  // Delete all entities
  for (auto &e : m_entities) {
    delete e;
  }
  m_entities.clear();
  m_idCounter = 0;

  // re-init systems
  initializeSystems();
}

Entity &ECSManager::createEntity() {
  Entity *e = new Entity(m_idCounter++);
  m_addEntities.push_back(e);
  return *e;
}

void ECSManager::addEntity(Entity *entity) { m_addEntities.push_back(entity); }

void ECSManager::addComponent(Entity &entity, Component *component) {
  m_addComponents.push_back(addComponent_t{entity, component});
}

void ECSManager::removeEntity(int entityID) {
  m_removeEntities.push_back(entityID);
}

void ECSManager::removeComponent(Entity &entity, ComponentTypeEnum component) {
  m_removeComponents.push_back(removeComponent_t{entity, component});
}

Entity *ECSManager::getEntity(int entityID) {
  for (auto &entity : m_entities) {
    if (entity->getID() == entityID) {
      return entity;
    }
  }
  return nullptr;
}

// PRIVATE

void ECSManager::addEntities() {
  for (auto &newEntity : m_addEntities) {
    // add to manager
    m_entities.push_back(newEntity);

    // add to systems
    for (auto &system : m_systems) {
      system.second->addEntity(m_entities.back());
    }
  }
  m_addEntities.clear();
}

void ECSManager::addComponents() {
  for (auto &components : m_addComponents) {
    // if enitity does not already have component, proceed
    if (components.ent.addComponent(components.cmp)) {
      for (auto &system : m_systems) {
        // if entity is not already belonging to the system, try and add it
        if (!system.second->containsEntity(components.ent.getID())) {
          system.second->addEntity(&components.ent);
        }
      }
    }
  }
  m_addComponents.clear();
}

void ECSManager::removeEntities() {
  for (auto &i : m_removeEntities) {
    // delete in systems
    for (auto &system : m_systems) {
      system.second->removeEntity(i);
    }

    // delete in manager
    for (unsigned int j = 0; j < m_entities.size(); j++) {
      if (m_entities[j]->getID() == i) {
        delete m_entities[j];
        m_entities.erase(m_entities.begin() + j);
      }
    }
  }
  m_removeEntities.clear();
}

void ECSManager::removeComponents() {
  for (auto &components : m_removeComponents) {
    components.ent.removeComponent(components.cmp);
    for (auto &system : m_systems) {
      system.second->removeFaultyEntity(components.ent.getID());
    }
  }
  m_removeComponents.clear();
}

const int ECSManager::createPlayerEntity(float x, float y, GLFWwindow *window) {
  Entity &playerEntity = createEntity();
  playerEntity.setName("Player");
  playerEntity.makePlayable();
  // Add components to player
  GraphicsComponent *graphComp = new GraphicsComponent();
  MeshShaderProgram *p = new MeshShaderProgram;
  Mesh *m = new Mesh(*p);
  m->LoadFlile("resources/Models/gltf/helmet/DamagedHelmet.glb");
  // m->LoadFlile("resources/Models/gltf/sponza/Sponza.gltf");
  graphComp->grapObj = m;

  addComponent(playerEntity, graphComp);
  PositionComponent *posComp = new PositionComponent();
  posComp->rotation = 30.0f;
  addComponent(playerEntity, posComp);
  return playerEntity.getID();
}
