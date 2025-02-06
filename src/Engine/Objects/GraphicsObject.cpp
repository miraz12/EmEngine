#include "GraphicsObject.hpp"

void
GraphicsObject::newNode(glm::mat4 model)
{
  p_numNodes = 1;
  p_nodes = std::make_unique<Node[]>(p_numNodes);
  p_nodes[0].mesh = 0;
  p_nodes[0].nodeMat = model;
}

glm::mat4
GraphicsObject::getLocalMat(i32 node)
{
  return glm::translate(glm::mat4(1.0f), p_nodes[node].trans) *
         glm::toMat4(p_nodes[node].rot) *
         glm::scale(glm::mat4(1.0f), p_nodes[node].scale) *
         p_nodes[node].nodeMat;
}

glm::mat4
GraphicsObject::getMatrix(i32 node)
{
  // If the node has no parent, return its local matrix
  if (p_nodes[node].parent == -1) {
    return getLocalMat(node);
  } else {
    // Recursively combine the parent transformation with the local matrix
    return getMatrix(p_nodes[node].parent) * getLocalMat(node);
  }
}

void
GraphicsObject::applySkinning(const ShaderProgram& sPrg, i32 node)
{

  std::vector<glm::mat4> jointMat;
  for (u32 j = 0; j < p_skins[node].joints.size(); j++) {
    i32 joint = p_skins[node].joints[j];
    std::string name = p_nodes[joint].name;
    jointMat.push_back(getMatrix(joint) * p_skins[node].inverseBindMatrices[j]);
  }

  if (!jointMat.empty()) {

    p_textureManager.bindTexture("jointMats");
    glTexImage2D(GL_TEXTURE_2D,
                 0,                            // mipmap level
                 GL_RGBA32F,                   // internal format
                 4,                            // width (4 columns per matrix)
                 jointMat.size(),              // height (number of matrices)
                 0,                            // border
                 GL_RGBA,                      // format
                 GL_FLOAT,                     // type
                 glm::value_ptr(jointMat[0])); // data

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
}

void
GraphicsObject::draw(const ShaderProgram& sPrg)
{

  for (u32 i = 0; i < p_numNodes; i++) {
    if (p_nodes[i].mesh >= 0) {
      if (p_nodes[i].skin >= 0) {
        glUniform1f(sPrg.getUniformLocation("is_skinned"), 1.0f);
        applySkinning(sPrg, p_nodes[i].skin);
      } else {
        glUniform1f(sPrg.getUniformLocation("is_skinned"), 0.0f);
      }

      Mesh& m = p_meshes[p_nodes[i].mesh];
      for (u32 j = 0; j < m.numPrims; j++) {
        Material* mat = m.m_primitives[j].m_material > -1
                          ? &p_materials[m.m_primitives[j].m_material]
                          : &defaultMat;

        mat->bind(sPrg);
        m.m_primitives[j].draw();
      }
    }
  }
}
void
GraphicsObject::drawGeom(const ShaderProgram&)
{
  for (u32 i = 0; i < p_numNodes; i++) {
    if (p_nodes[i].mesh >= 0) {
      // if (p_nodes[i].skin >= 0) {
      //   glUniform1f(sPrg.getUniformLocation("is_skinned"), 1.0f);
      //   applySkinning(sPrg, p_nodes[i].skin);
      // } else {
      //   glUniform1f(sPrg.getUniformLocation("is_skinned"), 0.0f);
      // }

      Mesh& m = p_meshes[p_nodes[i].mesh];
      for (u32 i = 0; i < m.numPrims; i++) {
        m.m_primitives[i].draw();
      }
    }
  }
}
