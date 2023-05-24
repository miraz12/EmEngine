#include "TextureManager.hpp"

#include <cassert>
#include <iostream>
#include <stb_image.h>
#include <string>



unsigned int TextureManager::loadTexture(unsigned int internalFormat, GLenum format, GLenum type,
                                         unsigned int width, unsigned int height,
                                         unsigned char *data) {
  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
  // glGenerateMipmap(GL_TEXTURE_2D);

  std::string name = std::to_string(texId);
  deleteTexture(name);
  texIds.insert({name, texId});
  return texId;
}
unsigned int TextureManager::loadTexture(std::string name, unsigned int internalFormat,
                                         GLenum format, GLenum type, unsigned int width,
                                         unsigned int height, unsigned char *data) {
  std::cout << "Warning texture reload" << std::endl;
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
  // glGenerateMipmap(GL_TEXTURE_2D);
  texIds.insert({name, texId});
  return texId;
}

void TextureManager::setTexture(std::string name, unsigned int texId) {
  deleteTexture(name);
  texIds.insert({name, texId});
}

unsigned int TextureManager::bindTexture(std::string name) {
  unsigned int texId = texIds.at(name);
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
