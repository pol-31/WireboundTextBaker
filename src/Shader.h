#ifndef WIREBOUNDTEXTBAKER__SHADER_H_
#define WIREBOUNDTEXTBAKER__SHADER_H_

#include <string>
#include <string_view>

#include <glad/glad.h>

class Shader {
 public:
  Shader() = default;
  /// in current program we don't need other shaders
  Shader(std::string_view vert_path, std::string_view frag_path);

  ~Shader();

  Shader(const Shader& other) = delete;
  Shader& operator=(const Shader& other) = delete;

  Shader(Shader&& other) noexcept;
  Shader& operator=(Shader&& other) noexcept;

  // technically it's not const, because internal data
  // can be affected by other OpenGL functions
  void Bind();

  // technically they're not const, because they affect internal OpenGL
  // shader representation (by opengl_id_)

  inline void SetUniform(const GLchar* name, GLboolean v0) {
    glUniform1f(glGetUniformLocation(opengl_id_, name),
                static_cast<float>(v0));
  }

  inline void SetUniform(const GLchar* name, GLfloat v0) {
    glUniform1f(glGetUniformLocation(opengl_id_, name), v0);
  }

  inline void SetUniform(const GLchar* name, GLfloat v0, GLfloat v1) {
    glUniform2f(glGetUniformLocation(opengl_id_, name), v0, v1);
  }

  inline void SetUniform(const GLchar* name, GLfloat v0,
                         GLfloat v1, GLfloat v2) {
    glUniform3f(glGetUniformLocation(opengl_id_, name), v0, v1, v2);
  }

  inline void SetUniform(const GLchar* name, GLfloat v0,
                         GLfloat v1, GLfloat v2, GLfloat v3) {
    glUniform4f(glGetUniformLocation(opengl_id_, name), v0, v1, v2, v3);
  }

  inline void SetUniform(const GLchar* name, GLint v0) {
    glUniform1i(glGetUniformLocation(opengl_id_, name), v0);

  }

  inline void SetUniform(const GLchar* name, GLint v0, GLint v1) {
    glUniform2i(glGetUniformLocation(opengl_id_, name), v0, v1);

  }

  inline void SetUniform(const GLchar* name, GLint v0, GLint v1, GLint v2) {
    glUniform3i(glGetUniformLocation(opengl_id_, name), v0, v1, v2);

  }

  inline void SetUniform(const GLchar* name, GLint v0, GLint v1,
                         GLint v2, GLint v3) {
    glUniform4i(glGetUniformLocation(opengl_id_, name), v0, v1, v2, v3);

  }

  inline void SetUniform(const GLchar* name, GLuint v0) {
    glUniform1ui(glGetUniformLocation(opengl_id_, name), v0);

  }

  inline void SetUniform(const GLchar* name, GLuint v0, GLuint v1) {
    glUniform2ui(glGetUniformLocation(opengl_id_, name), v0, v1);

  }

  inline void SetUniform(const GLchar* name, GLuint v0, GLuint v1, GLuint v2) {
    glUniform3ui(glGetUniformLocation(opengl_id_, name), v0, v1, v2);

  }

  inline void SetUniform(const GLchar* name, GLuint v0, GLuint v1,
                         GLuint v2, GLuint v3) {
    glUniform4ui(glGetUniformLocation(opengl_id_, name), v0, v1, v2, v3);

  }

  inline void SetUniformVec1(const GLchar* name, GLsizei count,
                             const GLfloat* value) {
    glUniform1fv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec2(const GLchar* name, GLsizei count,
                             const GLfloat* value) {
    glUniform2fv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec3(const GLchar* name, GLsizei count,
                             const GLfloat* value) {
    glUniform3fv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec4(const GLchar* name, GLsizei count,
                             const GLfloat* value) {
    glUniform4fv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec1(const GLchar* name, GLsizei count,
                             const GLint* value) {
    glUniform1iv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec2(const GLchar* name, GLsizei count,
                             const GLint* value) {
    glUniform2iv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec3(const GLchar* name, GLsizei count,
                             const GLint* value) {
    glUniform3iv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec4(const GLchar* name, GLsizei count,
                             const GLint* value) {
    glUniform4iv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec1(const GLchar* name, GLsizei count,
                             const GLuint* value) {
    glUniform1uiv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec2(const GLchar* name, GLsizei count,
                             const GLuint* value) {
    glUniform2uiv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec3(const GLchar* name, GLsizei count,
                             const GLuint* value) {
    glUniform3uiv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformVec4(const GLchar* name, GLsizei count,
                             const GLuint* value) {
    glUniform4uiv(glGetUniformLocation(opengl_id_, name), count, value);

  }

  inline void SetUniformMat2v(const GLchar* name, GLsizei count,
                              GLboolean transpose, const GLfloat* value) {
    glUniformMatrix2fv(glGetUniformLocation(opengl_id_, name),
                       count, transpose, value);
  }

  inline void SetUniformMat3v(const GLchar* name, GLsizei count,
                              GLboolean transpose, const GLfloat* value) {
    glUniformMatrix3fv(glGetUniformLocation(opengl_id_, name),
                       count, transpose, value);
  }

  inline void SetUniformMat4v(const GLchar* name, GLsizei count,
                              GLboolean transpose, const GLfloat* value) {
    glUniformMatrix4fv(glGetUniformLocation(opengl_id_, name),
                       count, transpose, value);
  }

  inline void SetUniformMat2x3v(const GLchar* name, GLsizei count,
                                GLboolean transpose, const GLfloat* value) {
    glUniformMatrix2x3fv(glGetUniformLocation(opengl_id_, name),
                         count, transpose, value);
  }

  inline void SetUniformMat3x2v(const GLchar* name, GLsizei count,
                                GLboolean transpose, const GLfloat* value) {
    glUniformMatrix3x2fv(glGetUniformLocation(opengl_id_, name),
                         count, transpose, value);
  }

  inline void SetUniformMat2x4v(const GLchar* name, GLsizei count,
                                GLboolean transpose, const GLfloat* value) {
    glUniformMatrix2x4fv(glGetUniformLocation(opengl_id_, name),
                         count, transpose, value);
  }

  inline void SetUniformMat4x2v(const GLchar* name, GLsizei count,
                                GLboolean transpose, const GLfloat* value) {
    glUniformMatrix4x2fv(glGetUniformLocation(opengl_id_, name),
                         count, transpose, value);
  }

  inline void SetUniformMat3x4v(const GLchar* name, GLsizei count,
                                GLboolean transpose, const GLfloat* value) {
    glUniformMatrix3x4fv(glGetUniformLocation(opengl_id_, name),
                         count, transpose, value);
  }

  inline void SetUniformMat4x3v(const GLchar* name, GLsizei count,
                                GLboolean transpose, const GLfloat* value) {
    glUniformMatrix4x3fv(glGetUniformLocation(opengl_id_, name),
                         count, transpose, value);
  }

 private:
  class ShaderObject {
   public:
    ShaderObject() = default;
    ShaderObject(const std::string& path, GLenum type);
    ~ShaderObject();

    ShaderObject(const ShaderObject& other) = delete;
    ShaderObject& operator=(const ShaderObject& other) = delete;

    ShaderObject(ShaderObject&& other) noexcept;
    ShaderObject& operator=(ShaderObject&& other) noexcept;

    // technically it's not const, because opengl_id_ is internal OpenGL id
    // which can be affected by OpenGL functions
    GLuint GetId() {
      return opengl_id_;
    }

   private:
    GLuint opengl_id_{0};
  };

  void Init(std::string_view vert_path, std::string_view frag_path);

  static std::string LoadShader(std::string_view path);

  static void CheckCompilation(GLuint shader, GLenum type);
  static void CheckLinking(GLuint shader);

  static std::string ShaderNameFromType(GLenum type);

  GLuint opengl_id_{0};
};

#endif //WIREBOUNDTEXTBAKER__SHADER_H_
