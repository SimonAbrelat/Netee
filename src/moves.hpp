#ifndef MOVES_HPP
#define MOVES_HPP

struct move {
    short input_direction;
    short rapier_direction;
};

move attack[] {
    // STARTUP: 3 frames 
    // rapier +30
    {0, 12}, {0, 9}, {0, 9},

    // ACTIVE: 3 frames
    {0, 0}, {0, 0}, {0, 0},

    // RECOVERY: 6 frames
    // rapier -30
    {0, -4}, {0, -4}, {0, -4}, {0, -4}, {0, -4}, {0, -4}, {0, -4}, {0, -2}
};
int ATTACK_COUNT = sizeof attack / sizeof *attack;

#endif