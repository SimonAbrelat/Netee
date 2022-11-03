#include "graphics.hpp"
#include "states.hpp"

#include <iostream>

Graphics::Graphics() {
    _win = SDL_CreateWindow("SDL2 Window",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                680, 480,
                                0);

    if(!_win) {
        std::cout << "Failed to create window\n";
        std::cout << "SDL2 Error: " << SDL_GetError() << "\n";
        return;
    }

    _win_surf = SDL_GetWindowSurface(_win);

    if(!_win_surf) {
        std::cout << "Failed to get window's surface\n";
        std::cout << "SDL2 Error: " << SDL_GetError() << "\n";
        return;
    }
}

Graphics::~Graphics() {
    SDL_FreeSurface(_win_surf);
    SDL_DestroyWindow(_win);
}

void Graphics::update(std::shared_ptr<Physics> phys) {
    bool keep_window_open = true;
    InputState input {};
    while(keep_window_open) {
        while(SDL_PollEvent(&_win_event) > 0) {
            switch(_win_event.type ){
                case SDL_KEYDOWN:
                    switch(_win_event.key.keysym.sym ){
                    case SDLK_LEFT:
                        input.direction = -1;
                        break;
                    case SDLK_RIGHT:
                        input.direction = 1;
                        break;
                    case SDLK_ESCAPE:
                        keep_window_open = false;
                        break;
                    default:
                        input.direction = 0;
                        break;
                    }
                    break;
                case SDL_QUIT:
                    input.direction = 0;
                    keep_window_open = false;
                    break;
                default:
                    input.direction = 0;
                    break;
            }
        }

        phys->update_inputs(input);
        PlayerState p1 = phys->get(true);
        int x = static_cast<int>(p1.pos);
        //std::cout << "Player 1 position: " << x << '\n';
        draw();
    }
}

void Graphics::draw() {
    SDL_UpdateWindowSurface(_win);
}
