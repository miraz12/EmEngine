#include "SceneLoader.hpp"
#include <ECS/Components/ParticlesComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <Objects/Cube.hpp>
#include <Objects/GltfObject.hpp>
#include <Objects/Heightmap.hpp>
#include <Objects/Line.hpp>
#include <yaml-cpp/yaml.h>

void SceneLoader::init(std::string sceneFile) {
  YAML::Node config = YAML::LoadFile(sceneFile);
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
        ECSManager::getInstance().SetupPointLight(glm::vec3(r, g, b), constant,
                                                  linear, quadratic,
                                                  glm::vec3(x, y, z));
      } else if (dict["type"].as<std::string>() == "dir") {
        float r = dict["color"][0].as<float>();
        float g = dict["color"][1].as<float>();
        float b = dict["color"][2].as<float>();
        float ambient = dict["ambient"].as<float>();
        float x = dict["direction"][0].as<float>();
        float y = dict["direction"][1].as<float>();
        float z = dict["direction"][2].as<float>();
        ECSManager::getInstance().SetupDirectionalLight(
            glm::vec3(r, g, b), ambient, glm::vec3(x, y, z));
      }
    }
    if (dict["entity"]) {
      YAML::Node n = dict["entity"];
      Entity en = ECSManager::getInstance().createEntity();
      if (dict["components"]) {
        YAML::Node components = dict["components"];

        for (size_t i = 0; i < components.size(); i++) {
          if (components[i]["type"].as<std::string>() == "Gra") {
            std::shared_ptr<GraphicsComponent> graphComp;
            if (components[i]["primitive"].as<std::string>() == "Cube") {
              graphComp = std::make_shared<GraphicsComponent>(*new Cube());
            } else if (components[i]["primitive"].as<std::string>() == "Quad") {
              graphComp = std::make_shared<GraphicsComponent>(*new Quad());
            } else if (components[i]["primitive"].as<std::string>() == "Line") {
              // TODO: Fix this when needed
              // graphComp = std::make_shared<GraphicsComponent>(*new Line());
            } else if (components[i]["primitive"].as<std::string>() ==
                       "Point") {
              // TODO: Fix this when needed
              // graphComp = std::make_shared<GraphicsComponent>(*new Point());
            } else if (components[i]["primitive"].as<std::string>() == "Mesh") {
              graphComp = std::make_shared<GraphicsComponent>(
                  *new GltfObject("resources/Models/" +
                                  components[i]["file"].as<std::string>()));
            } else if (components[i]["primitive"].as<std::string>() ==
                       "Heightmap") {
              graphComp = std::make_shared<GraphicsComponent>(
                  *new Heightmap("resources/Textures/" +
                                 components[i]["file"].as<std::string>()));
            }
            ECSManager::getInstance().addComponents(en, graphComp);

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
            ECSManager::getInstance().addComponents(en, posComp);

          } else if (components[i]["type"].as<std::string>() == "Phy") {
            std::shared_ptr<PhysicsComponent> physComp;
            if (components[i]["mass"]) {
              physComp = std::make_shared<PhysicsComponent>(
                  en, components[i]["mass"].as<float>());
            } else {
              physComp = std::make_shared<PhysicsComponent>(en, 0.0f);
            }
            ECSManager::getInstance().addComponents(en, physComp);
          } else if (components[i]["type"].as<std::string>() == "Par") {
            float x = components[i]["position"][0].as<float>();
            float y = components[i]["position"][1].as<float>();
            float z = components[i]["position"][2].as<float>();

            float xv = components[i]["velocity"][0].as<float>();
            float yv = components[i]["velocity"][1].as<float>();
            float zv = components[i]["velocity"][2].as<float>();
            std::shared_ptr<ParticlesComponent> parComp =
                std::make_shared<ParticlesComponent>(glm::vec3(x, y, z),
                                                     glm::vec3(xv, yv, zv));
            ECSManager::getInstance().addComponents(en, parComp);
          }
        }
      }
    }
  }
}