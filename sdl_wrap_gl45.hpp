#pragma once
#include "glad/glad.h"
#include "sdl_wrap_header.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <array>
#include <fstream>
#include <vector>

void setRequiredGlVersion(int major = 4, int minor = 5,
                          int profile = SDL_GL_CONTEXT_PROFILE_CORE) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile);
}

class GlContext {
    SDL_GLContext context_;

  public:
    GlContext(SDL_Window *window, bool vsync = true)
        : context_(SDL_GL_CreateContext(window)) {
        SDL_GL_MakeCurrent(window, context_);
        if (vsync)
            SDL_GL_SetSwapInterval(1);
        gladLoadGLLoader(SDL_GL_GetProcAddress);
        printf("Loaded OpenGL version %s\n", glGetString(GL_VERSION));
        assert(glGetError() == GL_NO_ERROR, "failed to create GL context");
    }
};

class GlObject {
  protected:
    GLuint id_;

  public:
    GlObject() = default;
    ~GlObject() = default;
    DEF_COPY_MOVE(GlObject, delete)
    virtual operator GLuint() { return id_; }
};

class VertexArray : public GlObject {
  public:
    VertexArray() {
        glGenVertexArrays(1, &id_);
        glBindVertexArray(id_);
    }
    ~VertexArray() { glDeleteVertexArrays(1, &id_); }
};

class Buffer : public GlObject {
    GLenum target_ = GL_NONE;

  public:
    template <class T, size_t N>
    Buffer(GLenum target, std::array<T, N> &data,
           GLenum usage = GL_STATIC_DRAW) {
        glGenBuffers(1, &id_);
        bind(target);
        setData(data, usage);
    }
    ~Buffer() { glDeleteBuffers(1, &id_); }

    /**
     * Rebind the buffer to another target.
     */
    void bind(GLenum target = GL_ARRAY_BUFFER) {
        target_ = target;
        glBindBuffer(target_, id_);
    }

    /**
     * Set the buffer's data to the given contents.
     */
    template <class T, size_t N>
    void setData(std::array<T, N> &data, GLenum usage = GL_STATIC_DRAW) {
        assert(target_ != GL_NONE, "must bind Buffer before filling with data");
        glBufferData(target_, N * sizeof(T), data.data(), usage);
    }
};

class Texture : public GlObject {
    GLenum type_;

  public:
    Texture(GLenum type = GL_TEXTURE_2D, GLenum wrap = GL_CLAMP_TO_BORDER)
        : type_(type) {
        glGenTextures(1, &id_);
        glBindTexture(type_, id_);
        for (auto c : {GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R})
            glTextureParameteri(type_, c, wrap);
    }

    void setBorderColor(std::array<GLfloat, 4> &color) {
        glTexParameterfv(type_, GL_TEXTURE_BORDER_COLOR, color.data()); 
    }
};

class Shader : public GlObject {
  public:
    Shader(GLenum type, std::string path) {
        std::ifstream file(path);
        id_ = glCreateShader(type);
        std::string result, line;
        while (std::getline(file, line))
            result += line + "\n";
        const auto pointer = result.c_str();
        glShaderSource(id_, 1, &pointer, nullptr);
        glCompileShader(id_);
        GLint logLength;
        glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::vector<char> log(size_t(logLength + 1));
            glGetShaderInfoLog(id_, logLength, nullptr, log.data());
            printf("Error when compiling shader %d: \n%s", id_, log.data());
        }
    }
    ~Shader() { glDeleteShader(id_); }
};

class Program : public GlObject {
    std::vector<GLuint> shaders;

  public:
    Program() { id_ = glCreateProgram(); }
    ~Program() {
        for (auto &s : shaders)
            glDetachShader(id_, s);
        glDeleteProgram(id_);
    }

    void attachShader(GLuint id) {
        glAttachShader(id_, id);
        shaders.push_back(id);
    }

    void link() {
        glLinkProgram(id_);
        GLint logLength;
        glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::vector<char> log(size_t(logLength + 1));
            glGetShaderInfoLog(id_, logLength, nullptr, log.data());
            printf("Error when linking program %d: %s", id_, log.data());
        }
    }

    /**
     * Set up the format for the given attribute array.
     * @param ident array to be modified (= a shader-internal variable)
     * @param dimension number of components (between 1 and 4)
     * @param type data type of the components (e.g. GL_FLOAT, GL_INT)
     * @param normalize enables normalization of the vector
     * @param byteStride bytes between the beginnings of two consecutive
     * attributes (0 = no space in between)
     * @param byteOffset bytes until the beginning of the first attribute
     * @return the attribute array "location" (= unique ID)
     */
    GLuint setAttribPointer(std::string ident, GLint dimension, GLenum type,
                            GLboolean normalize = GL_FALSE,
                            GLint byteStride = 0, void *byteOffset = nullptr) {
        auto attrib = GLuint(glGetAttribLocation(id_, ident.c_str()));
        glVertexAttribPointer(attrib, dimension, type, normalize, byteStride,
                              byteOffset);
        return attrib;
    }

    void enableAttrib(std::vector<GLuint> attribs) {
        for (auto a : attribs)
            glEnableVertexAttribArray(a);
    }
    void disableAttrib(std::vector<GLuint> attribs) {
        for (auto a : attribs)
            glDisableVertexAttribArray(a);
    }
};

GLenum err;

std::string errorString() {
    switch (err) {
    case GL_INVALID_ENUM:
        return "Invalid enum type";
    case GL_INVALID_OPERATION:
        return "Invalid operation/parameters";
    case GL_INVALID_VALUE:
        return "Invalid value/parameters";
    default:
        return "";
    }
}
