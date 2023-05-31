#include "TextureManager.hpp"

#include <cassert>
#include <iostream>
#include <stb_image.h>
#include <string>

uint32_t TextureManager::loadTexture(uint32_t internalFormat, GLenum format, GLenum type,
                                     uint32_t width, uint32_t height, unsigned char *data) {
  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  std::string name = std::to_string(texId);
  deleteTexture(name);
  texIds.insert({name, texId});
  return texId;
}
uint32_t TextureManager::loadTexture(std::string name, uint32_t internalFormat, GLenum format,
                                     GLenum type, uint32_t width, uint32_t height,
                                     unsigned char *data) {
  // std::cout << "Warning texture reload" << std::endl;
  deleteTexture(name);
  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  texIds.insert({name, texId});
  return texId;
}

void TextureManager::setTexture(std::string name, uint32_t texId) {
  deleteTexture(name);
  texIds.insert({name, texId});
}

uint32_t TextureManager::bindTexture(std::string name) {
  uint32_t texId = texIds.at(name);
  glBindTexture(GL_TEXTURE_2D, texId);
  return texId;
}

void TextureManager::bindCubeTexture(std::string name) {
  glBindTexture(GL_TEXTURE_CUBE_MAP, texIds.at(name));
}

void TextureManager::deleteTexture(std::string name) {
  auto iter = texIds.find(name);
  if (iter != texIds.end()) {
    glDeleteTextures(1, &iter->second);
    texIds.erase(name);
  }
}
