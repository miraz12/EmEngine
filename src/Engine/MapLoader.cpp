#include "MapLoader.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PhysicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "ECS/ECSManager.hpp"
#include "Objects/Cube.hpp"
#include "ResourceManager.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

void
MapLoader::loadMap(const std::string& filename, float tileSize)
{
  m_ecsMan = &ECSManager::getInstance();
  m_tileSize = tileSize;

  // Parse the map file
  parseMapFile(filename);

  // Generate entities from the map data
  generateMapEntities();

  // Create default directional light for shadows
  Entity lightEntity = m_ecsMan->createEntity("MapDirectionalLight");
  m_ecsMan->setupDirectionalLight(
    lightEntity,
    glm::vec3(1.0f, 1.0f, 1.0f),     // white light
    10.0f,                           // intensity
    glm::vec3(-0.3f, -1.0f, -0.3f)); // direction (downward + slight angle)
}

void
MapLoader::parseMapFile(const std::string& filename)
{
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Failed to open map file: " << filename << std::endl;
    return;
  }

  m_mapData.clear();
  std::string line;
  m_width = 0;
  m_height = 0;

  while (std::getline(file, line)) {
    if (!line.empty()) {
      m_mapData.push_back(line);
      m_width = std::max(m_width, static_cast<int>(line.length()));
      m_height++;
    }
  }

  file.close();

  std::cout << "Loaded map: " << m_width << "x" << m_height << " tiles"
            << std::endl;
}

void
MapLoader::generateMapEntities()
{
  if (m_mapData.empty()) {
    std::cerr << "No map data to generate entities from" << std::endl;
    return;
  }

  // Create floor first
  createFloor();

  // Create walls based on map data
  for (int z = 0; z < m_height; z++) {
    const std::string& row = m_mapData[z];
    for (int x = 0; x < static_cast<int>(row.length()); x++) {
      char tile = row[x];
      if (tile == 'X' || tile == 'x') {
        createWall(x, z);
      }
      // 'O' or 'o' or space means empty, so we skip
    }
  }
}

void
MapLoader::createWall(int x, int z)
{
  // Create entity for the wall
  std::stringstream entityName;
  entityName << "Wall_" << x << "_" << z;
  Entity wallEntity = m_ecsMan->createEntity(entityName.str());

  // Add graphics component with a cached cube
  ResourceManager& resourceMgr = ResourceManager::getInstance();
  auto& gc = m_ecsMan->emplaceComponent<GraphicsComponent>(
    wallEntity, resourceMgr.getCube());
  gc.type = GraphicsComponent::TYPE::CUBE;

  // Add position component
  float offsetX = -(m_width * m_tileSize) / 2.0f;
  float offsetZ = -(m_height * m_tileSize) / 2.0f;

  auto& pc = m_ecsMan->emplaceComponent<PositionComponent>(wallEntity);
  pc.position = glm::vec3(offsetX + x * m_tileSize + m_tileSize / 2.0f,
                          m_tileSize / 2.0f,
                          offsetZ + z * m_tileSize + m_tileSize / 2.0f);
  pc.scale = glm::vec3(m_tileSize, m_tileSize, m_tileSize);
  pc.rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));

  m_ecsMan->emplaceComponent<PhysicsComponent>(
    wallEntity, wallEntity, 0.0f, CollisionShapeType::BOX);
}

void
MapLoader::createFloor()
{
  // Create entity for the floor
  Entity floorEntity = m_ecsMan->createEntity("MapFloor");

  // Add graphics component with a cached cube (scaled flat)
  ResourceManager& resourceMgr = ResourceManager::getInstance();
  auto& gc = m_ecsMan->emplaceComponent<GraphicsComponent>(
    floorEntity, resourceMgr.getCube());
  gc.type = GraphicsComponent::TYPE::CUBE;

  // Scale the floor to cover the entire map
  float floorWidth = m_width * m_tileSize;
  float floorDepth = m_height * m_tileSize;
  float floorHeight = 0.1f * m_tileSize;

  auto& pc = m_ecsMan->emplaceComponent<PositionComponent>(floorEntity);
  pc.position = glm::vec3(0.0f, 0.0f, 0.0f);
  pc.scale = glm::vec3(floorWidth, floorHeight, floorDepth);
  pc.rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f));

  m_ecsMan->emplaceComponent<PhysicsComponent>(
    floorEntity, floorEntity, 0.0f, CollisionShapeType::BOX);
}
