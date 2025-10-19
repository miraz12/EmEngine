#ifndef RESOURCEMANAGER_H_
#define RESOURCEMANAGER_H_

#include <Singleton.hpp>
#include <memory>
#include <string>
#include <unordered_map>

class GraphicsObject;
class Cube;
class Quad;
class GltfObject;
class Heightmap;

class ResourceManager : public Singleton<ResourceManager>
{
  friend class Singleton<ResourceManager>;

public:
  // Get a shared cube instance (cached singleton)
  std::shared_ptr<Cube> getCube();

  // Get a shared quad instance (cached singleton)
  std::shared_ptr<Quad> getQuad();

  // Get a cached GLTF model by filename
  // If not loaded, loads and caches it
  std::shared_ptr<GltfObject> getGltfModel(const std::string& filename);

  // Get a cached heightmap by filename
  // If not loaded, loads and caches it
  std::shared_ptr<Heightmap> getHeightmap(const std::string& filename);

  // Clear all cached resources
  void clearCache();

  // Get cache statistics
  size_t getModelCacheSize() const { return m_gltfCache.size(); }
  size_t getHeightmapCacheSize() const { return m_heightmapCache.size(); }

private:
  ResourceManager() = default;
  ~ResourceManager() = default;

  // Cached primitive shapes (singleton instances)
  std::shared_ptr<Cube> m_cube;
  std::shared_ptr<Quad> m_quad;

  // Cached models by filename
  std::unordered_map<std::string, std::shared_ptr<GltfObject>> m_gltfCache;

  // Cached heightmaps by filename
  std::unordered_map<std::string, std::shared_ptr<Heightmap>> m_heightmapCache;
};

#endif // RESOURCEMANAGER_H_
