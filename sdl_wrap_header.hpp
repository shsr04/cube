#pragma once
#include <cstdio>
#include <string>
#include <SDL2/SDL_error.h>

#define DEF_COPY(name, ident)                                                  \
    name(name const &) = ident;                                                \
    auto operator=(name const &) = ident;
#define DEF_MOVE(name, ident)                                                  \
    name(name &&) = ident;                                                     \
    name &operator=(name &&) = ident;
#define DEF_COPY_MOVE(name, ident) DEF_COPY(name, ident) DEF_MOVE(name, ident)

void assert(bool expr, std::string msg) {
    if (!expr) {
        fprintf(stderr, "%s: %s\n", msg.c_str(), SDL_GetError());
        std::terminate();
    }
}
