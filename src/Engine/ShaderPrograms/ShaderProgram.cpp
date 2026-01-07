#include "ShaderProgram.hpp"

#include <fstream>
#include <iostream>
#include <string>

ShaderProgram::ShaderProgram(std::string_view vertexShaderPath,
                             std::string_view fragmentShaderPath)
{
  loadShaders(vertexShaderPath, fragmentShaderPath);
}

ShaderProgram::~ShaderProgram()
{
  glDeleteProgram(m_shaderProgram);
}

void
ShaderProgram::setUniformBinding(const std::string& uni)
{
  m_uniformBindings[uni] = glGetUniformLocation(m_shaderProgram, uni.c_str());
}

void
ShaderProgram::setAttribBinding(const std::string& attr)
{
  m_attribBindings[attr] = glGetAttribLocation(m_shaderProgram, attr.c_str());
}

void
ShaderProgram::loadShaders(std::string_view vertexShaderPath,
                           std::string_view fragmentShaderPath)
{
  // vertex shader
  std::string vertexShaderString;
  readFile(vertexShaderPath, &vertexShaderString);
  const char* vertexShaderSrc = vertexShaderString.c_str();

  u32 vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
  glCompileShader(vertexShader);
  // check for shader compile errors
  i32 success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    std::cout << vertexShaderPath << '\n'
              << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << '\n';
  }

  // fragment shader
  std::string fragmentShaderString;
  readFile(fragmentShaderPath, &fragmentShaderString);
  const char* fragmentShaderSrc = fragmentShaderString.c_str();

  u32 fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
  glCompileShader(fragmentShader);
  // check for shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    std::cout << fragmentShaderPath << '\n'
              << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << '\n';
  }

  m_shaderProgram = glCreateProgram();
  glAttachShader(m_shaderProgram, vertexShader);
  glAttachShader(m_shaderProgram, fragmentShader);
  glLinkProgram(m_shaderProgram);

  // check for linking errors
  glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(m_shaderProgram, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << '\n';
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

void
ShaderProgram::use() const
{
  glUseProgram(m_shaderProgram);
}

u32
ShaderProgram::getUniformLocation(const std::string& uniformName) const
{
  if (const auto& iter = m_uniformBindings.find(uniformName);
      iter != m_uniformBindings.end()) {
    return iter->second;
  }
  std::cout << "No uniform with name " << uniformName << "\n";
  return 0;
}

u32
ShaderProgram::getAttribLocation(const std::string& attribName) const
{
  if (const auto& iter = m_attribBindings.find(attribName);
      iter != m_attribBindings.end()) {
    return iter->second;
  }
  std::cout << "No attribute with name " << attribName << "\n";
  return 0;
}

void
ShaderProgram::readFile(std::string_view filePath, std::string* result)
{
  std::string line;
  std::ifstream theFile(filePath.data());
  if (theFile.is_open()) {
    while (std::getline(theFile, line)) {
      result->append(line + "\n");
    }
    theFile.close();
  } else {
    std::cout << "Could not open file " << filePath << "\n";
  }
}
