#include "MeshObj.hpp"
#include <glm/gtc/type_ptr.hpp>

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

void MeshObj::draw(const ShaderProgram &sPrg) {

  glUniformMatrix4fv(sPrg.getUniformLocation("meshMatrix"), 1, GL_FALSE,
                     glm::value_ptr(m_meshMatrix));

  glBindVertexArray(m_vao);
  glDrawElements(m_mode, m_eboCount, m_eboType, (void *)(sizeof(char) * (m_eboOffset)));
  glBindVertexArray(0);

  // if (!m_positions.empty()) {
  //   loc = sPrg.getAttribLocation("POSITION");
  //   glBindBuffer(GL_ARRAY_BUFFER, m_posAttribInfo.vbo);
  //   glVertexAttribPointer(loc, m_posAttribInfo.type, m_posAttribInfo.componentType,
  //                         m_posAttribInfo.normalized, m_posAttribInfo.byteStride,
  //                         (void *)(sizeof(char) * (m_posAttribInfo.byteOffset)));
  //   glEnableVertexAttribArray(loc);
  // }
  // if (!m_normals.empty()) {
  //   loc = sPrg.getAttribLocation("NORMAL");
  //   glBindBuffer(GL_ARRAY_BUFFER, m_norAttribInfo.vbo);
  //   glVertexAttribPointer(loc, m_norAttribInfo.type, m_norAttribInfo.componentType,
  //                         m_norAttribInfo.normalized, m_norAttribInfo.byteStride,
  //                         (void *)(sizeof(char) * (m_norAttribInfo.byteOffset)));
  //   glEnableVertexAttribArray(loc);
  // }
  // if (!m_tangents.empty()) {
  //   loc = sPrg.getAttribLocation("TANGENT");
  //   glBindBuffer(GL_ARRAY_BUFFER, m_tanAttribInfo.vbo);
  //   glVertexAttribPointer(loc, m_tanAttribInfo.type, m_tanAttribInfo.componentType,
  //                         m_tanAttribInfo.normalized, m_tanAttribInfo.byteStride,
  //                         (void *)(sizeof(char) * (m_tanAttribInfo.byteOffset)));
  //   glEnableVertexAttribArray(loc);
  // }
  // if (!m_texCoords.empty()) {
  //   loc = sPrg.getAttribLocation("TEXCOORD_0");
  //   glBindBuffer(GL_ARRAY_BUFFER, m_texAttribInfo.vbo);
  //   glVertexAttribPointer(loc, m_texAttribInfo.type, m_texAttribInfo.componentType,
  //                         m_texAttribInfo.normalized, m_texAttribInfo.byteStride,
  //                         (void *)(sizeof(char) * (m_texAttribInfo.byteOffset)));
  //   glEnableVertexAttribArray(loc);
  // }

  // if (!m_indices.empty()) {
  //   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indices[0]);
  //   glDrawElements(m_mode, m_indices.size(), GL_UNSIGNED_INT, 0);
  // } else {
  //   // for (auto &attrib : primitive.attributes) {
  //   //   tinygltf::Accessor accessor = model.accessors[attrib.second];
  //   //   unsigned int loc = m_buffers.at(accessor.bufferView);
  //   //   glBindBuffer(GL_ARRAY_BUFFER, loc);
  //   //   int byteStride = accessor.ByteStride(model.bufferViews[accessor.bufferView]);
  //   //   glVertexAttribPointer(loc, accessor.type, accessor.componentType, accessor.normalized,
  //   //                         byteStride, (void *)(sizeof(char) * (accessor.byteOffset)));
  //   //   glEnableVertexAttribArray(loc);
  //   // }
  //   // tinygltf::Accessor indexAccessor = model.accessors[primitive.indices];
  //   // glBindBuffer(GL_ARRAY_BUFFER, m_buffers.at(indexAccessor.bufferView));
  //   // glDrawArrays(primitive.mode, 0, model.accessors[primitive.indices].count);
  // }
}
