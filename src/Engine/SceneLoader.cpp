#include "SceneLoader.hpp"
#include "ECS/Components/AnimationComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/ParticlesComponent.hpp"
#include "ECS/Components/PhysicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "ECS/ECSManager.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "Objects/Cube.hpp"
#include "Objects/GltfObject.hpp"
#include "Objects/Heightmap.hpp"
#include "Objects/Line.hpp"
#include "Objects/Quad.hpp"
#include "ResourceManager.hpp"
#include <ECS/Components/DebugComponent.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
          } else if (component["type"].as<std::string>() == "Cam") {
            addCameraComponent(entity, component);
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
      switch (ligComp->type) {
        case LightingComponent::TYPE::NONE:
          throw std::logic_error(
            "Cannot serialize LightingComponent with type NONE");
          break;
        case LightingComponent::TYPE::POINT: {
          auto* point = static_cast<PointLight*>(ligComp->light.get());
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
          auto* dir = static_cast<DirectionalLight*>(ligComp->light.get());
          out << YAML::Key << "lightType" << YAML::Value << "dir";
          out << YAML::Key << "color" << YAML::Value << YAML::Flow
              << YAML::BeginSeq << dir->color.r << dir->color.g << dir->color.b
              << YAML::EndSeq;
          out << YAML::Key << "direction" << YAML::Value << YAML::Flow
              << YAML::BeginSeq << dir->direction.x << dir->direction.y
              << dir->direction.z << YAML::EndSeq;
          out << YAML::Key << "ambient" << YAML::Value << dir->intensity;
          break;
      }
      out << YAML::EndMap;
    }
    auto parComp = ecsMan.getComponent<ParticlesComponent>(en);
    if (parComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Par";
      glm::vec3 vel = parComp->velocity;
      out << YAML::Key << "velocity" << YAML::Value << YAML::Flow
          << YAML::BeginSeq << vel.x << vel.y << vel.z << YAML::EndSeq;
      out << YAML::EndMap;
    }
    auto phyComp = ecsMan.getComponent<PhysicsComponent>(en);
    if (phyComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Phy";
      out << YAML::Key << "mass" << YAML::Value << phyComp->getMass();
      out << YAML::Key << "shape" << YAML::Value
          << static_cast<int>(phyComp->getShapeType());
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
  std::string prim = component["primitive"].as<std::string>();

  if (prim == "Cube") {
    auto& gc =
      ecsMan.emplaceComponent<GraphicsComponent>(entity, resourceMgr.getCube());
    gc.type = GraphicsComponent::TYPE::CUBE;
  } else if (prim == "Quad") {
    auto& gc =
      ecsMan.emplaceComponent<GraphicsComponent>(entity, resourceMgr.getQuad());
    gc.type = GraphicsComponent::TYPE::QUAD;
  } else if (prim == "Line") {
    std::cerr << "Warning: Line primitive not yet implemented, skipping"
              << std::endl;
    return;
  } else if (prim == "Point") {
    std::cerr << "Warning: Point primitive not yet implemented, skipping"
              << std::endl;
    return;
  } else if (prim == "Mesh") {
    std::string modelPath =
      "resources/Models/" + component["file"].as<std::string>();
    auto obj = resourceMgr.getGltfModel(modelPath);
    bool hasAnims = obj->p_numAnimations > 0;
    auto& gc =
      ecsMan.emplaceComponent<GraphicsComponent>(entity, std::move(obj));
    gc.type = GraphicsComponent::TYPE::MESH;
    if (hasAnims) {
      ecsMan.emplaceComponent<AnimationComponent>(entity);
    }
  } else if (prim == "Heightmap") {
    std::string heightmapPath =
      "resources/Textures/" + component["file"].as<std::string>();
    auto& gc = ecsMan.emplaceComponent<GraphicsComponent>(
      entity, resourceMgr.getHeightmap(heightmapPath));
    gc.type = GraphicsComponent::TYPE::HEIGHTMAP;
  } else {
    std::cerr << "Error: Unknown primitive type: " << prim << std::endl;
    return;
  }
}

void
SceneLoader::addPositionComponent(Entity entity, const YAML::Node& component)
{
  auto& ecsMan = ECSManager::getInstance();
  auto& posComp = ecsMan.emplaceComponent<PositionComponent>(entity);
  if (component["position"]) {
    auto x = component["position"][0].as<float>();
    auto y = component["position"][1].as<float>();
    auto z = component["position"][2].as<float>();
    posComp.position = glm::vec3(x, y, z);
  }
  if (component["scale"]) {
    auto x = component["scale"][0].as<float>();
    auto y = component["scale"][1].as<float>();
    auto z = component["scale"][2].as<float>();
    posComp.scale = glm::vec3(x, y, z);
  }
  if (component["rotation"]) {
    auto x = component["rotation"][0].as<float>();
    auto y = component["rotation"][1].as<float>();
    auto z = component["rotation"][2].as<float>();
    posComp.rotation = glm::quat(glm::vec3(x, y, z));
  }
}

void
SceneLoader::addPhysicsComponent(Entity entity, const YAML::Node& component)
{
  auto& ecsMan = ECSManager::getInstance();
  float mass = component["mass"] ? component["mass"].as<float>() : 0.0f;
  ecsMan.emplaceComponent<PhysicsComponent>(
    entity, entity, mass, CollisionShapeType(component["shape"].as<int>()));
}

void
SceneLoader::addParticlesComponent(Entity entity, const YAML::Node& component)
{
  auto& ecsMan = ECSManager::getInstance();
  auto xv = component["velocity"][0].as<float>();
  auto yv = component["velocity"][1].as<float>();
  auto zv = component["velocity"][2].as<float>();
  ecsMan.emplaceComponent<ParticlesComponent>(entity, glm::vec3(xv, yv, zv));
}
void
SceneLoader::addCameraComponent(Entity entity, const YAML::Node& component)
{
  auto& ecsMan = ECSManager::getInstance();

  float fov = component["fov"] ? component["fov"].as<float>() : 45.0f;
  float near = component["near"] ? component["near"].as<float>() : 0.1f;
  float far = component["far"] ? component["far"].as<float>() : 200.0f;
  float width = component["width"] ? component["width"].as<float>() : 1600.0f;
  float height =
    component["height"] ? component["height"].as<float>() : 1200.0f;
  bool isMain = component["main"] ? component["main"].as<bool>() : true;

  glm::vec3 position(0.0f, 0.0f, 5.0f);
  if (component["position"]) {
    position = glm::vec3(component["position"][0].as<float>(),
                         component["position"][1].as<float>(),
                         component["position"][2].as<float>());
  }

  glm::vec3 target(0.0f, 0.0f, 0.0f);
  if (component["target"]) {
    target = glm::vec3(component["target"][0].as<float>(),
                       component["target"][1].as<float>(),
                       component["target"][2].as<float>());
  }

  glm::vec3 up(0.0f, 1.0f, 0.0f);
  if (component["up"]) {
    up = glm::vec3(component["up"][0].as<float>(),
                   component["up"][1].as<float>(),
                   component["up"][2].as<float>());
  }

  auto& camComp = ecsMan.emplaceComponent<CameraComponent>(
    entity, fov, width, height, near, far);
  camComp.m_position = position;
  camComp.m_front = glm::normalize(target - position);
  camComp.m_viewMatrix = glm::lookAt(position, target, up);
  camComp.m_ProjectionMatrix =
    glm::perspective(glm::radians(fov), width / height, near, far);
  camComp.m_matrixNeedsUpdate = false;
  if (isMain) {
    CameraSystem::getInstance().setMainCamera(entity);
  }
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
