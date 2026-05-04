#include "ResourceManager.hpp"
#include "Objects/Cube.hpp"
#include "Objects/GltfObject.hpp"
#include "Objects/Heightmap.hpp"
#include "Objects/Quad.hpp"

std::shared_ptr<Cube>
ResourceManager::getCube()
{
  // Lazy initialization - create cube only once
  if (!m_cube) {
    m_cube = std::make_shared<Cube>();
  }
  return m_cube;
}

std::shared_ptr<Quad>
ResourceManager::getQuad()
{
  // Lazy initialization - create quad only once
  if (!m_quad) {
    m_quad = std::make_shared<Quad>();
  }
  return m_quad;
}

std::shared_ptr<GltfObject>
ResourceManager::getGltfModel(const std::string& filename)
{
  // Check if model is already cached
  auto it = m_gltfCache.find(filename);
  if (it != m_gltfCache.end()) {
    return it->second;
  }

  // Model not cached, load it
  auto model = std::make_shared<GltfObject>(filename);
  m_gltfCache[filename] = model;
  return model;
}

std::shared_ptr<Heightmap>
ResourceManager::getHeightmap(const std::string& filename)
{
  // Check if heightmap is already cached
  auto it = m_heightmapCache.find(filename);
  if (it != m_heightmapCache.end()) {
    return it->second;
  }

  // Heightmap not cached, load it
  auto heightmap = std::make_shared<Heightmap>(filename);
  m_heightmapCache[filename] = heightmap;
  return heightmap;
}

void
ResourceManager::clearCache()
{
  m_cube.reset();
  m_quad.reset();
  m_gltfCache.clear();
  m_heightmapCache.clear();
}
