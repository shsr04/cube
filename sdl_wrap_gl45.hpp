#pragma once
#include "glad/glad.h"
#include "sdl_wrap_header.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <fstream>
#include <vector>

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
    VertexArray() { glGenVertexArrays(1, &id_); }
    ~VertexArray() { glDeleteVertexArrays(1, &id_); }

    void bind() { glBindVertexArray(id_); }
};

class Buffer : public GlObject {
  public:
    Buffer() { glGenBuffers(1, &id_); }
    ~Buffer() { glDeleteBuffers(1, &id_); }

    void bind(GLenum target = GL_ARRAY_BUFFER) { glBindBuffer(target, id_); }
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
            std::vector<char> log(logLength + 1);
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
            std::vector<char> log(logLength + 1);
            glGetShaderInfoLog(id_, logLength, nullptr, log.data());
            printf("Error when linking program %d: %s", id_, log.data());
        }
    }
};

void setInitialGlAttributes(int major = 4, int minor = 5,
                            int doubleBuffer = 1) {
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, doubleBuffer);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
}

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
