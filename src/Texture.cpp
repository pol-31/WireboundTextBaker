#include "Texture.h"

#include <stdexcept>

#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "Details.h"

Texture::Texture(std::string_view path) {
  if (!path.empty()) {
    Init(path);
  }
}

Texture::Texture(int opengl_id) : opengl_id_(opengl_id) {}

void Texture::Init(std::string_view path) {
  glGenTextures(1, &opengl_id_);
  glBindTexture(GL_TEXTURE_2D, opengl_id_);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int channels;
  unsigned char* data = stbi_load(path.data(), &width_,
                                  &height_, &channels, 0);
  if (data) {
    switch (channels) {
      case 1:
        format_ = GL_RED;
        break;
      case 2:
        format_ = GL_RG;
        break;
      case 3:
        format_ = GL_RGB;
        break;
      default:
        format_ = GL_RGBA;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, format_, width_, height_, 0,
                 format_, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    throw std::runtime_error("Failed to load texture");
  }
  stbi_image_free(data);
}

Texture::Texture(Texture&& other) noexcept {
  glDeleteTextures(1, &opengl_id_);
  opengl_id_ = other.opengl_id_;
  width_ = other.width_;
  height_ = other.height_;
  format_ = other.format_;
  other.opengl_id_ = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
  glDeleteTextures(1, &opengl_id_);
  opengl_id_ = other.opengl_id_;
  width_ = other.width_;
  height_ = other.height_;
  format_ = other.format_;
  other.opengl_id_ = 0;
  return *this;
}

Texture::~Texture() {
  glDeleteTextures(1, &opengl_id_);
}

void Texture::Bind() {
  glBindTexture(GL_TEXTURE_2D, opengl_id_);
}
