#ifndef SHADERPROGRAM_H_
#define SHADERPROGRAM_H_

#include <string>
#include <unordered_map>

// Base shader class, inherit from this and define your own
// setupVertexAttributePointers that matches the shaders
class ShaderProgram
{
public:
  ShaderProgram() = delete;
  ShaderProgram(std::string_view vertexShaderPath,
                std::string_view fragmentShaderPath);
  virtual ~ShaderProgram();
  ShaderProgram(const ShaderProgram&) = delete;

  void setUniformBinding(const std::string& uni);
  void setAttribBinding(const std::string& attr);
  GLint getUniformLocation(const std::string& uniformName) const;
  GLint getAttribLocation(const std::string& attribName) const;
  u32 getId() const { return m_shaderProgram; }
  void loadShaders(std::string_view vertexShaderPath,
                   std::string_view fragmentShaderPath);
  void use() const;

private:
  u32 m_shaderProgram;
  std::unordered_map<std::string, GLint> m_uniformBindings;
  std::unordered_map<std::string, GLint> m_attribBindings;

  void readFile(std::string_view filePath, std::string* result);
};
#endif // SHADERPROGRAM_H_
