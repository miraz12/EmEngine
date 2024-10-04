#include "GraphicsObject.hpp"

void GraphicsObject::newNode(glm::mat4 model) {
  size_t idx = p_nodes.size();
  p_nodes.push_back(std::make_unique<Node>());
  p_nodes[idx]->mesh = 0;
  p_nodes[idx]->nodeMat = model;
}

glm::mat4 GraphicsObject::getLocalMat(i32 node) {
  return glm::translate(glm::mat4(1.0f), p_nodes[node]->trans) *
         glm::toMat4(p_nodes[node]->rot) *
         glm::scale(glm::mat4(1.0f), p_nodes[node]->scale) *
         p_nodes[node]->nodeMat;
}

glm::mat4 GraphicsObject::getMatrix(i32 node) {
  // If the node has no parent, return its local matrix
  if (p_nodes[node]->parent == -1) {
    return getLocalMat(node);
  } else {
    // Recursively combine the parent transformation with the local matrix
    return getMatrix(p_nodes[node]->parent) * getLocalMat(node);
  }
}

void GraphicsObject::draw(const ShaderProgram &sPrg) {

  for (u32 i = 0; i < p_nodes.size(); i++) {
    if (p_nodes[i]->mesh >= 0) {

      i32 skinNode = p_nodes[i]->skin;
      glm::mat4 result;
      if (skinNode >= 0) {
        glUniform1f(sPrg.getUniformLocation("is_skinned"), 1.0f);
        std::vector<glm::mat4> joinMat;
        for (u32 j = 0; j < p_skins[skinNode].joints.size(); j++) {
          i32 joint = p_skins[skinNode].joints[j];
          std::string name = p_nodes[joint]->name;
          result = getMatrix(joint) * p_skins[skinNode].inverseBindMatrices[j];
          joinMat.push_back(result);
        }

        if (!joinMat.empty()) {
          glUniformMatrix4fv(sPrg.getUniformLocation("jointMatrices"),
                             joinMat.size(), GL_FALSE, &joinMat[0][0][0]);
        }

      } else {
        glUniform1f(sPrg.getUniformLocation("is_skinned"), 0.0f);
      }

      Mesh &m = p_meshes[p_nodes[i]->mesh];
      for (u32 j = 0; j < m.numPrims; j++) {
        Material *mat = m.m_primitives[j].m_material > -1
                            ? &p_materials[m.m_primitives[j].m_material]
                            : &defaultMat;

        mat->bind(sPrg);
        m.m_primitives[j].draw();
      }
    }
  }
}
void GraphicsObject::drawGeom(const ShaderProgram &sPrg) {
  for (auto &n : p_nodes) {
    if (n->mesh >= 0) {
      glUniformMatrix4fv(sPrg.getUniformLocation("meshMatrix"), 1, GL_FALSE,
                         glm::value_ptr(n->nodeMat));
      Mesh &m = p_meshes[n->mesh];

      for (u32 i = 0; i < m.numPrims; i++) {
        m.m_primitives[i].draw();
      }
    }
  }
}
