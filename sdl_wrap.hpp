#pragma once
#include "sdl_wrap_gl.hpp"
#include "sdl_wrap_header.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <array>
#include <string>

class Init {
  public:
    Init() { SDL_Init(SDL_INIT_EVERYTHING); }
    ~Init() { SDL_Quit(); }
    DEF_COPY_MOVE(Init, delete);
};

class Window {
    SDL_Window *window_;

  public:
    const int width_, height_;

    Window(int width, int height, std::string title, Uint32 flags = 0)
        : window_(SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED, width, height,
                                   SDL_WINDOW_SHOWN | flags)),
          width_(width), height_(height) {}
    ~Window() { SDL_DestroyWindow(window_); }
    DEF_COPY_MOVE(Window, delete)

    operator SDL_Window *() { return window_; }
};

/**
 * SDL renderer, not to be used together with OpenGL rendering.
 */
class Renderer {
    SDL_Renderer *renderer_;

  public:
    Renderer(SDL_Window *window, Uint32 flags)
        : renderer_(SDL_CreateRenderer(window, -1, flags)) {}
    ~Renderer() { SDL_DestroyRenderer(renderer_); }
    DEF_COPY(Renderer, delete);
    DEF_MOVE(Renderer, default);

    operator SDL_Renderer *() { return renderer_; }

    void setColor(std::array<Uint8, 4> rgba) {
        SDL_SetRenderDrawColor(renderer_, rgba[0], rgba[1], rgba[2], rgba[3]);
    }

    void clear(std::array<Uint8, 4> rgba = {0, 0, 0, 0}) {
        setColor(rgba);
        SDL_RenderClear(renderer_);
    }

    void present() { SDL_RenderPresent(renderer_); }

    SDL_Texture *textureFrom(SDL_Surface *surface) {
        return SDL_CreateTextureFromSurface(renderer_, surface);
    }

    void copy(SDL_Texture *texture, SDL_Rect *srcArea = nullptr,
              SDL_Rect *destArea = nullptr) {
        SDL_RenderCopy(renderer_, texture, srcArea, destArea);
    }

    void fill(SDL_Rect *area = nullptr) { SDL_RenderFillRect(renderer_, area); }
    void draw(SDL_Rect *area = nullptr) { SDL_RenderDrawRect(renderer_, area); }
};

/**
 * ATTENTION: Renderer and Surface cannot be used together!
 * (Use Renderer instead)
 */
class Surface {
    SDL_Surface *surface_;

  public:
    Surface(SDL_Window *window) : surface_(SDL_GetWindowSurface(window)) {}
    ~Surface() { SDL_FreeSurface(surface_); }
    DEF_COPY(Surface, delete)
    DEF_MOVE(Surface, default)

    operator SDL_Surface *() { return surface_; }
};

/**
 * Don't forget to init the SDL2_image library:
 *      assert((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0, "PNG init failed");
 */
class Image {
    SDL_Surface *image_;

  public:
    Image(std::string path) : image_(IMG_Load(path.c_str())) {
        assert(image_, "PNG loading failed, did you forget IMG_Init?");
    }
    ~Image() { SDL_FreeSurface(image_); }
    DEF_COPY(Image, delete);
    DEF_MOVE(Image, default);

    operator SDL_Surface *() { return image_; }

    void convertTo(SDL_PixelFormat *format) {
        auto r = SDL_ConvertSurface(image_, format, 0);
        assert(r, "surface conversion failed");
        SDL_FreeSurface(image_);
        image_ = r;
    }
};
