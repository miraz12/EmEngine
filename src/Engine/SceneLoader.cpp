#include "SceneLoader.hpp"
#include "ECS/Components/ParticlesComponent.hpp"
#include "ECS/ECSManager.hpp"
#include "Objects/Cube.hpp"
#include "Objects/GltfObject.hpp"
#include "Objects/Heightmap.hpp"
#include "Objects/Line.hpp"
#include "Objects/Quad.hpp"
#include <ECS/Components/DebugComponent.hpp>

void SceneLoader::init(std::string sceneFile) {
  YAML::Node config = YAML::LoadFile(sceneFile);
  m_ecsMan = &ECSManager::getInstance();
  for (auto dict : config) {

    if (dict["light"]) {
      if (dict["type"].as<std::string>() == "point") {
        float r = dict["color"][0].as<float>();
        float g = dict["color"][1].as<float>();
        float b = dict["color"][2].as<float>();
        float constant = dict["constant"].as<float>();
        float linear = dict["linear"].as<float>();
        float quadratic = dict["quadratic"].as<float>();
        float x = dict["position"][0].as<float>();
        float y = dict["position"][1].as<float>();
        float z = dict["position"][2].as<float>();
        m_ecsMan->SetupPointLight(glm::vec3(r, g, b), constant, linear,
                                  quadratic, glm::vec3(x, y, z));
      } else if (dict["type"].as<std::string>() == "dir") {
        float r = dict["color"][0].as<float>();
        float g = dict["color"][1].as<float>();
        float b = dict["color"][2].as<float>();
        float ambient = dict["ambient"].as<float>();
        float x = dict["direction"][0].as<float>();
        float y = dict["direction"][1].as<float>();
        float z = dict["direction"][2].as<float>();
        m_ecsMan->SetupDirectionalLight(glm::vec3(r, g, b), ambient,
                                        glm::vec3(x, y, z));
      }
    }
    if (dict["entity"]) {
      YAML::Node n = dict["entity"];
      Entity en = m_ecsMan->createEntity();
      if (dict["components"]) {
        YAML::Node components = dict["components"];

        for (size_t i = 0; i < components.size(); i++) {
          if (components[i]["type"].as<std::string>() == "Gra") {
            std::shared_ptr<GraphicsComponent> graphComp;
            if (components[i]["primitive"].as<std::string>() == "Cube") {
              graphComp = std::make_shared<GraphicsComponent>(*new Cube());
              graphComp->type = GraphicsComponent::CUBE;
            } else if (components[i]["primitive"].as<std::string>() == "Quad") {
              graphComp = std::make_shared<GraphicsComponent>(*new Quad());
              graphComp->type = GraphicsComponent::QUAD;
            } else if (components[i]["primitive"].as<std::string>() == "Line") {
              // TODO: Fix this when needed
              // graphComp = std::make_shared<GraphicsComponent>(*new Line());
              graphComp->type = GraphicsComponent::LINE;
            } else if (components[i]["primitive"].as<std::string>() ==
                       "Point") {
              // TODO: Fix this when needed
              // graphComp = std::make_shared<GraphicsComponent>(*new Point());
              graphComp->type = GraphicsComponent::POINT;
            } else if (components[i]["primitive"].as<std::string>() == "Mesh") {
              graphComp = std::make_shared<GraphicsComponent>(
                  *new GltfObject("resources/Models/" +
                                  components[i]["file"].as<std::string>()));
              graphComp->type = GraphicsComponent::MESH;
            } else if (components[i]["primitive"].as<std::string>() ==
                       "Heightmap") {
              graphComp = std::make_shared<GraphicsComponent>(
                  *new Heightmap("resources/Textures/" +
                                 components[i]["file"].as<std::string>()));
              graphComp->type = GraphicsComponent::HEIGHTMAP;
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
              posComp->rotation = glm::vec3(x, y, z);
            }
            m_ecsMan->addComponents(en, posComp);

          } else if (components[i]["type"].as<std::string>() == "Phy") {
            std::shared_ptr<PhysicsComponent> physComp;
            if (components[i]["mass"]) {
              physComp = std::make_shared<PhysicsComponent>(
                  en, components[i]["mass"].as<float>());
            } else {
              physComp = std::make_shared<PhysicsComponent>(en, 0.0f);
            }
            m_ecsMan->addComponents(en, physComp);
          } else if (components[i]["type"].as<std::string>() == "Par") {
            float xv = components[i]["velocity"][0].as<float>();
            float yv = components[i]["velocity"][1].as<float>();
            float zv = components[i]["velocity"][2].as<float>();
            std::shared_ptr<ParticlesComponent> parComp =
                std::make_shared<ParticlesComponent>(glm::vec3(xv, yv, zv));
            m_ecsMan->addComponents(en, parComp);
          }
        }
      }
    }
  }
}

void SceneLoader::saveScene(std::string sceneFile) {
  YAML::Emitter out;
  std::vector<Entity> ents = m_ecsMan->getEntities();

  out << YAML::BeginSeq;
  for (const Entity &en : ents) {
    out << YAML::BeginMap;
    out << YAML::Key << "entity" << YAML::Value << en;
    out << YAML::Key << "components" << YAML::Value << YAML::BeginSeq;

    // Serialize each component of the entity
    auto posComp = m_ecsMan->getComponent<PositionComponent>(en);
    if (posComp) {
      out << YAML::BeginMap;
      out << YAML::Key << "type" << YAML::Value << "Pos";
      out << YAML::Key << "position" << YAML::Value << YAML::Flow
          << YAML::BeginSeq << posComp->position.x << posComp->position.y
          << posComp->position.z << YAML::EndSeq;
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
      case GraphicsComponent::POINT:
        out << YAML::Key << "primitive" << YAML::Value << "Point";
        break;
      case GraphicsComponent::LINE:
        out << YAML::Key << "primitive" << YAML::Value << "Line";
        break;
      case GraphicsComponent::QUAD:
        out << YAML::Key << "primitive" << YAML::Value << "Quad";
        break;
      case GraphicsComponent::CUBE:
        out << YAML::Key << "primitive" << YAML::Value << "Cube";
        break;
      case GraphicsComponent::HEIGHTMAP: {
        auto map = static_cast<Heightmap *>(&graComp->m_grapObj);
        out << YAML::Key << "primitive" << YAML::Value << "Heightmap";
        out << YAML::Key << "file" << YAML::Value
            << map->getFileName()
                   .substr(19, map->getFileName().length())
                   .data();
        break;
      }
      case GraphicsComponent::MESH:
        auto mesh = static_cast<GltfObject *>(&graComp->m_grapObj);
        out << YAML::Key << "primitive" << YAML::Value << "Mesh";
        out << YAML::Key << "file" << YAML::Value
            << mesh->getFileName()
                   .substr(17, mesh->getFileName().length())
                   .data();
        break;
      };
      out << YAML::EndMap;
    }
    // auto ligComp = m_ecsMan->getComponent<LightingComponent>(en);
    // if (ligComp) {
    //   out << YAML::BeginMap;
    //   out << YAML::Key << "type" << YAML::Value << "Gra";
    //   switch (ligComp->getType()) {
    //   case LightingComponent::POINT:
    //     break;
    //   case LightingComponent::DIRECTIONAL:
    //     break;
    //   }
    // }
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

  std::ofstream fout(sceneFile);
  fout << out.c_str();
}
