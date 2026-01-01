#ifndef MAPLOADER_H_
#define MAPLOADER_H_

#include <Singleton.hpp>
#include <string>
#include <vector>

class ECSManager;

class MapLoader : public Singleton<MapLoader>
{
  friend class Singleton<MapLoader>;

public:
  // Load a map from a text file
  // X = wall, O = empty space
  // tileSize controls the scale of each tile
  void loadMap(const std::string& filename, float tileSize = 1.0f);

  // Get dimensions of the loaded map
  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }
  float getTileSize() const { return m_tileSize; }

private:
  MapLoader() = default;
  ~MapLoader() = default;

  // Parse the map file and store the grid
  void parseMapFile(const std::string& filename);

  // Generate entities for walls and floor
  void generateMapEntities();

  // Create a wall cube at the specified position
  void createWall(int x, int z);

  // Create the floor plane
  void createFloor();

  ECSManager* m_ecsMan;
  std::vector<std::string> m_mapData;
  int m_width{ 0 };
  int m_height{ 0 };
  float m_tileSize{ 1.0f };
};

#endif // MAPLOADER_H_
