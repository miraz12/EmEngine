#include "LightingShaderProgram.hpp"

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

LightingShaderProgram::LightingShaderProgram()
    : ShaderProgram("resources/Shaders/lightVertex.glsl", "resources/Shaders/pbrLightFragment.glsl") {
  m_uniformBindings["gPositionAo"] = glGetUniformLocation(p_shaderProgram, "gPositionAo");
  m_uniformBindings["gNormalMetal"] = glGetUniformLocation(p_shaderProgram, "gNormalMetal");
  m_uniformBindings["gAlbedoSpecRough"] = glGetUniformLocation(p_shaderProgram, "gAlbedoSpecRough");
  m_uniformBindings["gEmissive"] = glGetUniformLocation(p_shaderProgram, "gEmissive");

  m_uniformBindings["nrOfPointLights"] = glGetUniformLocation(p_shaderProgram, "nrOfPointLights");
  m_uniformBindings["camPos"] = glGetUniformLocation(p_shaderProgram, "camPos");

  m_uniformBindings["directionalLight.direction"] =
      glGetUniformLocation(p_shaderProgram, "directionalLight.direction");
  m_uniformBindings["directionalLight.color"] =
      glGetUniformLocation(p_shaderProgram, "directionalLight.color");
  m_uniformBindings["directionalLight.ambientIntensity"] =
      glGetUniformLocation(p_shaderProgram, "directionalLight.ambientIntensity");

  for (unsigned int i = 0; i < 10; i++) {
    m_uniformBindings["pointLights[" + std::to_string(i) + "].position"] = glGetUniformLocation(
        p_shaderProgram, std::string("pointLights[" + std::to_string(i) + "].position").c_str());

    m_uniformBindings["pointLights[" + std::to_string(i) + "].color"] = glGetUniformLocation(
        p_shaderProgram, std::string("pointLights[" + std::to_string(i) + "].color").c_str());

    m_uniformBindings["pointLights[" + std::to_string(i) + "].diffuseIntensity"] =
        glGetUniformLocation(
            p_shaderProgram,
            std::string("pointLights[" + std::to_string(i) + "].diffuseIntensity").c_str());

    m_uniformBindings["pointLights[" + std::to_string(i) + "].constant"] = glGetUniformLocation(
        p_shaderProgram, std::string("pointLights[" + std::to_string(i) + "].constant").c_str());

    m_uniformBindings["pointLights[" + std::to_string(i) + "].linear"] = glGetUniformLocation(
        p_shaderProgram, std::string("pointLights[" + std::to_string(i) + "].linear").c_str());

    m_uniformBindings["pointLights[" + std::to_string(i) + "].quadratic"] = glGetUniformLocation(
        p_shaderProgram, std::string("pointLights[" + std::to_string(i) + "].quadratic").c_str());

    m_uniformBindings["pointLights[" + std::to_string(i) + "].radius"] = glGetUniformLocation(
        p_shaderProgram, std::string("pointLights[" + std::to_string(i) + "].radius").c_str());
  }

  use(); // Start using the shader
}
