#pragma once
#include "GraphicsObject.hpp"
#include <map>
#include <vector>
class Mesh : public GraphicsObject {
public:
  Mesh(std::string filename);
  virtual ~Mesh(){};
  void draw() override;

private:
  struct GLBufferState {
    unsigned int vb;
  };

  struct GLMeshState {
    std::vector<unsigned int> diffuseTex; // for each primitive in mesh
  };

  struct GLProgramState {
    std::map<std::string, int> attribs;
    std::map<std::string, int> uniforms;
  };

  struct GLCurvesState {
    unsigned int vb; // vertex buffer
    size_t count;    // byte count
  };

  std::map<int, GLBufferState> gBufferState;
  std::map<std::string, GLMeshState> gMeshState;
  std::map<int, GLCurvesState> gCurvesMesh;
};
