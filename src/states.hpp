#ifndef STATES_HPP
#define STATES_HPP

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

using f16 = fpm::fixed_16_16;

struct GameState {
    long frame;
    f16 p1_x;
    f16 p1_anim;
    f16 p2_x;
    f16 p2_anim;
};

struct InputState {
    short direction;
    bool attack;
    bool parry;
    bool feint;
    bool lunge;

    /*
    InputState() :
        direction(0),
        attack(false),
        parry(false),
        feint(false),
        lunge(false)
    {};
    */
};


#endif // STATES_HPP
