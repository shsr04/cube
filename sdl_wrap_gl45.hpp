#pragma once
#include "glad/glad.h"
#include "sdl_wrap_header.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_video.h>
#include <array>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
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
        assert_true(glGetError() == GL_NO_ERROR, "failed to create GL context");
    }
};

class GlObject {
  protected:
    GLuint id_;
    GlObject() = default;
    ~GlObject() = default;

  public:
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
        assert_true(target_ != GL_NONE,
                    "must bind Buffer before filling with data");
        glBufferData(target_, N * sizeof(T), data.data(), usage);
    }
};

class Texture : public GlObject {
    GLenum type_;

  public:
    /**
     * Attention: texture loading is not working!
     * Just don't do it.
     */
    Texture(GLenum type = GL_TEXTURE_2D, GLint wrap = GL_CLAMP_TO_BORDER)
        : type_(type) {
        glGenTextures(1, &id_);
        glBindTexture(type_, id_);
        glTexParameterf(type_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(type_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        for (auto c : {GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R})
            glTexParameteri(type_, GLenum(c), wrap);
    }
    ~Texture() { glDeleteTextures(1, &id_); }

    void setBorderColor(std::array<GLfloat, 4> &color) {
        glTexParameterfv(type_, GL_TEXTURE_BORDER_COLOR, color.data());
    }

    template <size_t W, size_t H>
    void loadRGB(std::array<GLubyte, 3 * W * H> pixels) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, W, H, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, pixels.data());
    }

    void loadImage(SDL_Surface *image) {
        assert_true(image->format->format == SDL_PIXELFORMAT_RGB24,
                    "Texture image must be in RGB24 format");
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->w, image->h, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, image->pixels);
        printf("loaded %dx%d image\n", image->w, image->h);
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
    GLint maxAttribs_;

  public:
    Program() {
        id_ = glCreateProgram();
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs_);
    }
    ~Program() { glDeleteProgram(id_); }

    void attachShader(GLuint id) { glAttachShader(id_, id); }

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
     * @param byteStride byte distance from start of attribute n to start of
     * attribute n+1 (0 = no space in between)
     * @param byteOffset bytes until the beginning of the first attribute
     * @return the attribute array "location" (= unique ID)
     */
    GLuint setAttribPointer(std::string ident, GLint dimension, GLenum type,
                            GLboolean normalize = GL_FALSE,
                            GLint byteStride = 0, size_t byteOffset = 0) {
        auto attrib = glGetAttribLocation(id_, ident.c_str());
        assert_true(attrib != -1,
                    "Attribute " + ident +
                        " not found (maybe optimized out because unused?)");
        assert_true(attrib < maxAttribs_, "Too many attributes specified");
        glVertexAttribPointer(GLuint(attrib), dimension, type, normalize,
                              byteStride, (void *)byteOffset);
        return GLuint(attrib);
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
