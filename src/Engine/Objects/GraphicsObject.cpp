#include "GraphicsObject.hpp"
#include <iostream>
#include <ostream>

void GraphicsObject::draw(const ShaderProgram &sPrg) {
  for (auto &n : p_nodes) {
    glUniformMatrix4fv(sPrg.getUniformLocation("meshMatrix"), 1, GL_FALSE,
                       glm::value_ptr(n->nodeMat));
    Mesh &m = p_meshes[n->mesh];
    for (u32 i = 0; i < m.numPrims; i++) {
      Material *mat = m.m_primitives[i].m_material > -1
                          ? &p_materials[m.m_primitives[i].m_material]
                          : &defaultMat;
      mat->bind(sPrg);
      m.m_primitives[i].draw();
    }
  }
}
void GraphicsObject::drawGeom(const ShaderProgram &sPrg) {
  for (auto &n : p_nodes) {
    glUniformMatrix4fv(sPrg.getUniformLocation("meshMatrix"), 1, GL_FALSE,
                       glm::value_ptr(n->nodeMat));
    Mesh &m = p_meshes[n->mesh];

    for (u32 i = 0; i < m.numPrims; i++) {
      m.m_primitives[i].draw();
    }
  }
}

GraphicsObject::~GraphicsObject() {
  for (auto &n : p_nodes) {
    delete n;
  }
}
