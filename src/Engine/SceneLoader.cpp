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
#include <ECS/Components/DebugComponent.hpp>
#include <ECS/ECSManager.hpp>

void SceneLoader::init(const char *file) {
  YAML::Node config = YAML::LoadFile(file);
  m_ecsMan = &ECSManager::getInstance();
  for (auto dict : config) {
    YAML::Node n = dict["entity"];
    if (n) {
      Entity en = m_ecsMan->createEntity(n.as<std::string>());
      if (dict["components"]) {
        YAML::Node components = dict["components"];
        for (size_t i = 0; i < components.size(); i++) {
          if (components[i]["type"].as<std::string>() == "Gra") {
            std::shared_ptr<GraphicsComponent> graphComp;
            if (components[i]["primitive"].as<std::string>() == "Cube") {
              graphComp =
                  std::make_shared<GraphicsComponent>(std::shared_ptr<Cube>());
              graphComp->type = GraphicsComponent::TYPE::CUBE;
            } else if (components[i]["primitive"].as<std::string>() == "Quad") {
              graphComp =
                  std::make_shared<GraphicsComponent>(std::shared_ptr<Quad>());
              graphComp->type = GraphicsComponent::TYPE::QUAD;
            } else if (components[i]["primitive"].as<std::string>() == "Line") {
              // TODO: Fix this when needed
              // graphComp = std::make_shared<GraphicsComponent>(*new Line());
              graphComp->type = GraphicsComponent::TYPE::LINE;
            } else if (components[i]["primitive"].as<std::string>() ==
                       "Point") {
              // TODO: Fix this when needed
              // graphComp = std::make_shared<GraphicsComponent>(*new Point());
              graphComp->type = GraphicsComponent::TYPE::POINT;
            } else if (components[i]["primitive"].as<std::string>() == "Mesh") {
              graphComp = std::make_shared<GraphicsComponent>(
                  std::make_shared<GltfObject>(
                      "resources/Models/" +
                      components[i]["file"].as<std::string>()));
              graphComp->type = GraphicsComponent::TYPE::MESH;
              if (graphComp->m_grapObj->p_numAnimations > 0) {
                std::shared_ptr<AnimationComponent> animComp =
                    std::make_shared<AnimationComponent>();
                m_ecsMan->addComponents(en, animComp);
              }
            } else if (components[i]["primitive"].as<std::string>() ==
                       "Heightmap") {
              std::string name = "resources/Textures/" +
                                 components[i]["file"].as<std::string>();
              graphComp = std::make_shared<GraphicsComponent>(
                  std::make_shared<Heightmap>(name));
              graphComp->type = GraphicsComponent::TYPE::HEIGHTMAP;
            }
            m_ecsMan->addComponents(en, graphComp);

          } else if (components[i]["type"].as<std::string>() == "Pos") {
            std::shared_ptr<PositionComponent> posComp =
                std::make_shared<PositionComponent>();
            if (components[i]["position"]) {
              float x = components[i]["position"][0].as<float>();
              float y = components[i]["position"][1].as<float>();
              float z = components[i]["position"][2].as<float>();
              posComp->position = glm::vec3(x, y, z);
            }
            if (components[i]["scale"]) {
              float x = components[i]["scale"][0].as<float>();
              float y = components[i]["scale"][1].as<float>();
              float z = components[i]["scale"][2].as<float>();
              posComp->scale = glm::vec3(x, y, z);
            }
            if (components[i]["rotation"]) {
              float x = components[i]["rotation"][0].as<float>();
              float y = components[i]["rotation"][1].as<float>();
              float z = components[i]["rotation"][2].as<float>();
              posComp->rotation = glm::quat(glm::vec3(x, y, z));
            }
            m_ecsMan->addComponents(en, posComp);

          } else if (components[i]["type"].as<std::string>() == "Phy") {
            std::shared_ptr<PhysicsComponent> physComp;
            if (components[i]["mass"]) {
              physComp = std::make_shared<PhysicsComponent>(
                  en, components[i]["mass"].as<float>(),
                  CollisionShapeType(components[i]["shape"].as<int>()));
            } else {
              physComp = std::make_shared<PhysicsComponent>(
                  en, 0.0f,
                  CollisionShapeType(components[i]["shape"].as<int>()));
            }
            m_ecsMan->addComponents(en, physComp);
          } else if (components[i]["type"].as<std::string>() == "Par") {
            float xv = components[i]["velocity"][0].as<float>();
            float yv = components[i]["velocity"][1].as<float>();
            float zv = components[i]["velocity"][2].as<float>();
            std::shared_ptr<ParticlesComponent> parComp =
                std::make_shared<ParticlesComponent>(glm::vec3(xv, yv, zv));
            m_ecsMan->addComponents(en, parComp);
          } else if (components[i]["type"].as<std::string>() == "Lig") {
            if (components[i]["lightType"].as<std::string>() == "point") {
              float r = components[i]["color"][0].as<float>();
              float g = components[i]["color"][1].as<float>();
              float b = components[i]["color"][2].as<float>();
              float constant = components[i]["constant"].as<float>();
              float linear = components[i]["linear"].as<float>();
              float quadratic = components[i]["quadratic"].as<float>();
              float x = components[i]["position"][0].as<float>();
              float y = components[i]["position"][1].as<float>();
              float z = components[i]["position"][2].as<float>();
              m_ecsMan->SetupPointLight(en, glm::vec3(r, g, b), constant,
                                        linear, quadratic, glm::vec3(x, y, z));
            } else if (components[i]["lightType"].as<std::string>() == "dir") {
              float r = components[i]["color"][0].as<float>();
              float g = components[i]["color"][1].as<float>();
              float b = components[i]["color"][2].as<float>();
              float ambient = components[i]["ambient"].as<float>();
              float x = components[i]["direction"][0].as<float>();
              float y = components[i]["direction"][1].as<float>();
              float z = components[i]["direction"][2].as<float>();
              m_ecsMan->SetupDirectionalLight(en, glm::vec3(r, g, b), ambient,
                                              glm::vec3(x, y, z));
            }
          }
        }
      }
    }
  }
}

void SceneLoader::saveScene(const char *file) {
  YAML::Emitter out;
  std::vector<Entity> ents = m_ecsMan->getEntities();

  out << YAML::BeginSeq;
  for (const Entity &en : ents) {
    out << YAML::BeginMap;
    out << YAML::Key << "entity" << YAML::Value
        << m_ecsMan->getEntityName(en).data();
    out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;

    // Serialize each component of the entity
    auto posComp = m_ecsMan->getComponent<PositionComponent>(en);
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
    auto debComp = m_ecsMan->getComponent<DebugComponent>(en);
    if (debComp) {
    }
    auto graComp = m_ecsMan->getComponent<GraphicsComponent>(en);
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
    auto ligComp = m_ecsMan->getComponent<LightingComponent>(en);
    if (ligComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Lig";
      switch (ligComp->getType()) {
      case LightingComponent::TYPE::NONE:
        throw;
        break;
      case LightingComponent::TYPE::POINT: {
        auto point = static_cast<PointLight *>(&ligComp->getBaseLight());
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
        auto dir = static_cast<DirectionalLight *>(&ligComp->getBaseLight());
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
    auto parComp = m_ecsMan->getComponent<ParticlesComponent>(en);
    if (parComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Par";
      glm::vec3 vel = parComp->getVelocity();
      out << YAML::Key << "velocity" << YAML::Value << YAML::Flow
          << YAML::BeginSeq << vel.x << vel.y << vel.z << YAML::EndSeq;
      out << YAML::EndMap;
    }
    auto phyComp = m_ecsMan->getComponent<PhysicsComponent>(en);
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
