#include "SceneLoader.hpp"
#include "ECS/Components/AnimationComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/ParticlesComponent.hpp"
#include "ECS/Components/PhysicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "ECS/ECSManager.hpp"
#include "Objects/Cube.hpp"
#include "Objects/GltfObject.hpp"
#include "Objects/Heightmap.hpp"
#include "Objects/Line.hpp"
#include "Objects/Quad.hpp"
#include "ResourceManager.hpp"
#include <ECS/Components/DebugComponent.hpp>

void
SceneLoader::init(const char* file)
{
  for (auto dict : YAML::LoadFile(file)) {
    if (YAML::Node node = dict["entity"]; node) {
      Entity entity =
        ECSManager::getInstance().createEntity(node.as<std::string>());
      if (dict["components"]) {
        for (const auto& component : dict["components"]) {
          if (component["type"].as<std::string>() == "Gra") {
            addGraphicsComponent(entity, component);
          } else if (component["type"].as<std::string>() == "Pos") {
            addPositionComponent(entity, component);
          } else if (component["type"].as<std::string>() == "Phy") {
            addPhysicsComponent(entity, component);
          } else if (component["type"].as<std::string>() == "Par") {
            addParticlesComponent(entity, component);
          } else if (component["type"].as<std::string>() == "Lig") {
            addDirectionalLight(entity, component);
          }
        }
      }
    }
  }
}

void
SceneLoader::saveScene(const char* file)
{
  YAML::Emitter out;
  auto& ecsMan = ECSManager::getInstance();
  std::vector<Entity> ents = ecsMan.getEntities();

  out << YAML::BeginSeq;
  for (const Entity& en : ents) {
    out << YAML::BeginMap;
    out << YAML::Key << "entity" << YAML::Value
        << ecsMan.getEntityName(en).data();
    out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;

    // Serialize each component of the entity
    auto posComp = ecsMan.getComponent<PositionComponent>(en);
    if (posComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Pos";
      out << YAML::Key << "position" << YAML::Value << YAML::Flow
          << YAML::BeginSeq << posComp->position.x << posComp->position.y
          << posComp->position.z << YAML::EndSeq;
      glm::vec3 rot = glm::eulerAngles(posComp->rotation);
      out << YAML::Key << "rotation" << YAML::Value << YAML::Flow
          << YAML::BeginSeq << rot.x << rot.y << rot.z << YAML::EndSeq;
      out << YAML::Key << "scale" << YAML::Value << YAML::Flow << YAML::BeginSeq
          << posComp->scale.x << posComp->scale.y << posComp->scale.z
          << YAML::EndSeq;
      out << YAML::EndMap;
    }
    auto debComp = ecsMan.getComponent<DebugComponent>(en);
    if (debComp) {
    }
    auto graComp = ecsMan.getComponent<GraphicsComponent>(en);
    if (graComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Gra";
      switch (graComp->type) {
        case GraphicsComponent::TYPE::POINT:
          out << YAML::Key << "primitive" << YAML::Value << "Point";
          break;
        case GraphicsComponent::TYPE::LINE:
          out << YAML::Key << "primitive" << YAML::Value << "Line";
          break;
        case GraphicsComponent::TYPE::QUAD:
          out << YAML::Key << "primitive" << YAML::Value << "Quad";
          break;
        case GraphicsComponent::TYPE::CUBE:
          out << YAML::Key << "primitive" << YAML::Value << "Cube";
          break;
        case GraphicsComponent::TYPE::HEIGHTMAP: {
          auto map = std::static_pointer_cast<Heightmap>(graComp->m_grapObj);
          out << YAML::Key << "primitive" << YAML::Value << "Heightmap";
          out << YAML::Key << "file" << YAML::Value
              << map->getFileName()
                   .substr(19, map->getFileName().length())
                   .data();
          break;
        }
        case GraphicsComponent::TYPE::MESH:
          auto mesh = std::static_pointer_cast<GltfObject>(graComp->m_grapObj);
          out << YAML::Key << "primitive" << YAML::Value << "Mesh";
          out << YAML::Key << "file" << YAML::Value
              << mesh->getFileName()
                   .substr(17, mesh->getFileName().length())
                   .data();
          break;
      };
      out << YAML::EndMap;
    }
    auto ligComp = ecsMan.getComponent<LightingComponent>(en);
    if (ligComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Lig";
      switch (ligComp->getType()) {
        case LightingComponent::TYPE::NONE:
          throw;
          break;
        case LightingComponent::TYPE::POINT: {
          auto* point = static_cast<PointLight*>(&ligComp->getBaseLight());
          out << YAML::Key << "lightType" << YAML::Value << "point";
          out << YAML::Key << "color" << YAML::Value << YAML::Flow
              << YAML::BeginSeq << point->color.r << point->color.g
              << point->color.b << YAML::EndSeq;
          out << YAML::Key << "position" << YAML::Value << YAML::Flow
              << YAML::BeginSeq << point->position.x << point->position.y
              << point->position.z << YAML::EndSeq;
          out << YAML::Key << "constant" << YAML::Value << point->constant;
          out << YAML::Key << "quadratic" << YAML::Value << point->quadratic;
          out << YAML::Key << "linear" << YAML::Value << point->linear;
          break;
        }
        case LightingComponent::TYPE::DIRECTIONAL:
          auto* dir = static_cast<DirectionalLight*>(&ligComp->getBaseLight());
          out << YAML::Key << "lightType" << YAML::Value << "dir";
          out << YAML::Key << "color" << YAML::Value << YAML::Flow
              << YAML::BeginSeq << dir->color.r << dir->color.g << dir->color.b
              << YAML::EndSeq;
          out << YAML::Key << "direction" << YAML::Value << YAML::Flow
              << YAML::BeginSeq << dir->direction.x << dir->direction.y
              << dir->direction.z << YAML::EndSeq;
          out << YAML::Key << "ambient" << YAML::Value << dir->ambientIntensity;
          break;
      }
      out << YAML::EndMap;
    }
    auto parComp = ecsMan.getComponent<ParticlesComponent>(en);
    if (parComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Par";
      glm::vec3 vel = parComp->getVelocity();
      out << YAML::Key << "velocity" << YAML::Value << YAML::Flow
          << YAML::BeginSeq << vel.x << vel.y << vel.z << YAML::EndSeq;
      out << YAML::EndMap;
    }
    auto phyComp = ecsMan.getComponent<PhysicsComponent>(en);
    if (phyComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Phy";
      out << YAML::Key << "mass" << YAML::Value << phyComp->getMass();
      out << YAML::EndMap;
    }

    // Repeat the above for other component types like GraphicsComponent,
    // PhysicsComponent, etc.
    out << YAML::EndSeq;
    out << YAML::EndMap;
  }
  out << YAML::EndSeq;

  std::ofstream fout(file);
  fout << out.c_str();
}

void
SceneLoader::addGraphicsComponent(Entity entity, const YAML::Node& component)
{
  auto& ecsMan = ECSManager::getInstance();
  auto& resourceMgr = ResourceManager::getInstance();
  std::shared_ptr<GraphicsComponent> graphComp;
  if (component["primitive"].as<std::string>() == "Cube") {
    graphComp = std::make_shared<GraphicsComponent>(resourceMgr.getCube());
    graphComp->type = GraphicsComponent::TYPE::CUBE;
  } else if (component["primitive"].as<std::string>() == "Quad") {
    graphComp = std::make_shared<GraphicsComponent>(resourceMgr.getQuad());
    graphComp->type = GraphicsComponent::TYPE::QUAD;
  } else if (component["primitive"].as<std::string>() == "Line") {
    // TODO: Fix this when needed
    // graphComp = std::make_shared<GraphicsComponent>(*new Line());
    // graphComp->type = GraphicsComponent::TYPE::LINE;
    std::cerr << "Warning: Line primitive not yet implemented, skipping"
              << std::endl;
    return;
  } else if (component["primitive"].as<std::string>() == "Point") {
    // TODO: Fix this when needed
    // graphComp = std::make_shared<GraphicsComponent>(*new Point());
    // graphComp->type = GraphicsComponent::TYPE::POINT;
    std::cerr << "Warning: Point primitive not yet implemented, skipping"
              << std::endl;
    return;
  } else if (component["primitive"].as<std::string>() == "Mesh") {
    std::string modelPath =
      "resources/Models/" + component["file"].as<std::string>();
    graphComp =
      std::make_shared<GraphicsComponent>(resourceMgr.getGltfModel(modelPath));
    graphComp->type = GraphicsComponent::TYPE::MESH;
    if (graphComp->m_grapObj->p_numAnimations > 0) {
      std::shared_ptr<AnimationComponent> animComp =
        std::make_shared<AnimationComponent>();
      ecsMan.addComponents(entity, animComp);
    }
  } else if (component["primitive"].as<std::string>() == "Heightmap") {
    std::string heightmapPath =
      "resources/Textures/" + component["file"].as<std::string>();
    graphComp = std::make_shared<GraphicsComponent>(
      resourceMgr.getHeightmap(heightmapPath));
    graphComp->type = GraphicsComponent::TYPE::HEIGHTMAP;
  } else {
    std::cerr << "Error: Unknown primitive type: "
              << component["primitive"].as<std::string>() << std::endl;
    return;
  }
  ecsMan.addComponents(entity, graphComp);
}

void
SceneLoader::addPositionComponent(Entity entity, const YAML::Node& component)
{
  auto& ecsMan = ECSManager::getInstance();
  auto posComp = std::make_shared<PositionComponent>();
  if (component["position"]) {
    auto x = component["position"][0].as<float>();
    auto y = component["position"][1].as<float>();
    auto z = component["position"][2].as<float>();
    posComp->position = glm::vec3(x, y, z);
  }
  if (component["scale"]) {
    auto x = component["scale"][0].as<float>();
    auto y = component["scale"][1].as<float>();
    auto z = component["scale"][2].as<float>();
    posComp->scale = glm::vec3(x, y, z);
  }
  if (component["rotation"]) {
    auto x = component["rotation"][0].as<float>();
    auto y = component["rotation"][1].as<float>();
    auto z = component["rotation"][2].as<float>();
    posComp->rotation = glm::quat(glm::vec3(x, y, z));
  }
  ecsMan.addComponents(entity, posComp);
}

void
SceneLoader::addPhysicsComponent(Entity entity, const YAML::Node& component)
{
  auto& ecsMan = ECSManager::getInstance();
  std::shared_ptr<PhysicsComponent> physComp;
  if (component["mass"]) {
    physComp = std::make_shared<PhysicsComponent>(
      entity,
      component["mass"].as<float>(),
      CollisionShapeType(component["shape"].as<int>()));
  } else {
    physComp = std::make_shared<PhysicsComponent>(
      entity, 0.0f, CollisionShapeType(component["shape"].as<int>()));
  }
  ecsMan.addComponents(entity, physComp);
}

void
SceneLoader::addParticlesComponent(Entity entity, const YAML::Node& component)
{
  auto& ecsMan = ECSManager::getInstance();
  auto xv = component["velocity"][0].as<float>();
  auto yv = component["velocity"][1].as<float>();
  auto zv = component["velocity"][2].as<float>();
  std::shared_ptr<ParticlesComponent> parComp =
    std::make_shared<ParticlesComponent>(glm::vec3(xv, yv, zv));
  ecsMan.addComponents(entity, parComp);
}
void
SceneLoader::addDirectionalLight(Entity entity, const YAML::Node& component)
{
  auto& ecsMan = ECSManager::getInstance();
  if (component["lightType"].as<std::string>() == "point") {
    auto r = component["color"][0].as<float>();
    auto g = component["color"][1].as<float>();
    auto b = component["color"][2].as<float>();
    auto constant = component["constant"].as<float>();
    auto linear = component["linear"].as<float>();
    auto quadratic = component["quadratic"].as<float>();
    auto x = component["position"][0].as<float>();
    auto y = component["position"][1].as<float>();
    auto z = component["position"][2].as<float>();
    ecsMan.setupPointLight(entity,
                           glm::vec3(r, g, b),
                           constant,
                           linear,
                           quadratic,
                           glm::vec3(x, y, z));
  } else if (component["lightType"].as<std::string>() == "dir") {
    auto r = component["color"][0].as<float>();
    auto g = component["color"][1].as<float>();
    auto b = component["color"][2].as<float>();
    auto ambient = component["ambient"].as<float>();
    auto x = component["direction"][0].as<float>();
    auto y = component["direction"][1].as<float>();
    auto z = component["direction"][2].as<float>();
    ecsMan.setupDirectionalLight(
      entity, glm::vec3(r, g, b), ambient, glm::vec3(x, y, z));
  }
}
