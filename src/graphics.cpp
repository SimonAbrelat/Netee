#include "graphics.hpp"
#include "states.hpp"

#include <iostream>

Graphics::Graphics() {
    SDL_Init(SDL_INIT_EVERYTHING);
    _win = SDL_CreateWindow("SDL2 Window",
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            680, 480,
                            0);

    if (!_win) {
        std::cout << "Failed to create window\n";
        std::cout << "SDL2 Error: " << SDL_GetError() << "\n";
        return;
    }

    _ren = SDL_CreateRenderer(_win, -1, SDL_RENDERER_ACCELERATED);

    _win_surf = SDL_GetWindowSurface(_win);

    if (!_win_surf) {
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
    InputState input{};
    while (keep_window_open) {
        while (SDL_PollEvent(&_win_event) > 0) {
            switch (_win_event.type) {
            case SDL_QUIT:
                input.direction = 0;
                keep_window_open = false;
                break;
            default:
                input.direction = 0;
                break;
            }
        }

        // INPUT DIRECTION
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_LEFT] && state[SDL_SCANCODE_RIGHT]) {
            input.direction = 0;
        }
        else if (state[SDL_SCANCODE_LEFT]) {
            input.direction = -5;
        }
        else if (state[SDL_SCANCODE_RIGHT]) {
            input.direction = 5;
        }

        if (state[SDL_SCANCODE_Q]) {
            input.attack = true;
        } else {
            input.attack = false;
        }

        // CLOSE WINDOW
        if (state[SDL_SCANCODE_ESCAPE]) {
            keep_window_open = false;
        }

        phys->update_inputs(input);
        PlayerState p1 = phys->get(true);
        int x = static_cast<int>(p1.pos);
        // std::cout << "Player 1 position: " << x << '\n';
        draw(phys);
    }
}

void Graphics::draw(std::shared_ptr<Physics> phys) {
    SDL_RenderClear(_ren);
    PlayerState p1 = phys->get(true);
    PlayerState p2 = phys->get(false);
    SDL_Rect p1_rect;
    SDL_Rect p2_rect;
    SDL_Rect p1_rapier_rect;
    SDL_Rect p2_rapier_rect;

    p1_rect.x = static_cast<int>(p1.pos);
    p1_rect.y = 100;
    p1_rect.w = 50;
    p1_rect.h = 50;

    p1_rapier_rect.x = static_cast<int>(p1.r_pos);
    p1_rapier_rect.y = 100;
    p1_rapier_rect.w = 100;
    p1_rapier_rect.h = 10;

    p2_rect.x = static_cast<int>(p2.pos);
    p2_rect.y = 100;
    p2_rect.w = 50;
    p2_rect.h = 50;

    p2_rapier_rect.x = static_cast<int>(p2.r_pos);
    p2_rapier_rect.y = 100;
    p2_rapier_rect.w = 100;
    p2_rapier_rect.h = 10;

    SDL_SetRenderDrawColor(_ren, 255, 4, 45, 255);
    SDL_RenderFillRect(_ren, &p1_rapier_rect);

    SDL_SetRenderDrawColor(_ren, 210, 4, 45, 255);
    SDL_RenderFillRect(_ren, &p1_rect);

    SDL_SetRenderDrawColor(_ren, 255, 0, 255, 200);
    SDL_RenderFillRect(_ren, &p2_rapier_rect);

    SDL_SetRenderDrawColor(_ren, 0, 0, 255, 200);
    SDL_RenderFillRect(_ren, &p2_rect);


    SDL_SetRenderDrawColor(_ren, 181, 126, 220, 255);

    SDL_RenderPresent(_ren);
}
