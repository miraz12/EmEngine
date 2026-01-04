#include "TextureManager.hpp"

u32
TextureManager::loadTexture(u32 internalFormat,
                            GLenum format,
                            GLenum type,
                            u32 width,
                            u32 height,
                            unsigned char* data)
{
  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(
    GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  std::string name = std::to_string(texId);
  deleteTexture(name);
  texIds.insert({ name, Texture{ texId, GL_TEXTURE_2D } });
  return texId;
}
u32
TextureManager::loadTexture(std::string name,
                            u32 internalFormat,
                            GLenum format,
                            GLenum type,
                            u32 width,
                            u32 height,
                            unsigned char* data)
{
  deleteTexture(name);
  GLuint texId;
  glGenTextures(1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(
    GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  texIds.insert({ name, Texture{ texId, GL_TEXTURE_2D } });
  return texId;
}

void
TextureManager::setTexture(std::string name, u32 texId, u32 type)
{
  deleteTexture(name);
  texIds.insert({ name, Texture{ texId, type } });
}

u32
TextureManager::bindTexture(std::string name)
{
  auto it = texIds.find(name);
  if (it == texIds.end()) {
    // Create a default 1x1 black texture if it doesn't exist
    std::cerr << "Warning: Texture '" << name
              << "' not found, creating default black texture" << std::endl;
    unsigned char blackPixel[4] = { 0, 0, 0, 255 };
    u32 texId =
      loadTexture(name, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, 1, 1, blackPixel);
    glBindTexture(GL_TEXTURE_2D, texId);
    return texId;
  }
  Texture& tex = it->second;
  glBindTexture(tex.type, tex.id);
  return tex.id;
}

u32
TextureManager::bindActivateTexture(std::string name, u32 pos)
{
  glActiveTexture(GL_TEXTURE0 + pos);
  return bindTexture(name);
}

void
TextureManager::deleteTexture(std::string name)
{
  auto iter = texIds.find(name);
  if (iter != texIds.end()) {
    glDeleteTextures(1, &iter->second.id);
    texIds.erase(name);
  }
}
