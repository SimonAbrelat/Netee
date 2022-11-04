#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <SDL2/SDL.h>

#include <memory>

#include "physics.hpp"

class Graphics {
public:
    Graphics();
    ~Graphics();

    void update(std::shared_ptr<Physics> phys);
    void draw(std::shared_ptr<Physics> phys);
private:
    SDL_Window  *_win;
    SDL_Renderer *_ren;
    SDL_Surface *_win_surf;
    SDL_Event    _win_event;
};

#endif // GRAPHICS
