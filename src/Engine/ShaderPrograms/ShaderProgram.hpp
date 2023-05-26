#ifndef SHADERPROGRAM_H_
#define SHADERPROGRAM_H_

#include <string>
#include <unordered_map>

// Base shader class, inherit from this and define your own
// setupVertexAttributePointers that matches the shaders
class ShaderProgram {
public:
  ShaderProgram() = delete;
  ShaderProgram(std::string vertexShaderPath, std::string fragmentShaderPath);
  virtual ~ShaderProgram();
  ShaderProgram(const ShaderProgram &) = delete;

  void setUniformBinding(std::string u);
  void setAttribBinding(std::string a);
  uint32_t getUniformLocation(std::string uniformName) const;
  uint32_t getAttribLocation(std::string attribName) const;
  uint32_t getId() { return p_shaderProgram; }
  void loadShaders(std::string vertexShaderPath, std::string fragmentShaderPath);
  void use() const;

protected:
  uint32_t p_shaderProgram;
  std::unordered_map<std::string, uint32_t> m_uniformBindings;
  std::unordered_map<std::string, int32_t> m_attribBindings;

private:
  void readFile(std::string filePath, std::string *result);
};
#endif // SHADERPROGRAM_H_
