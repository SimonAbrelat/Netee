#ifndef MOVES_HPP
#define MOVES_HPP

// TODO: Should these be f16s???
struct Move {
    short input_direction;
    short sword_direction;
};

const Move ATTACK_ANIMATION[] {
    // STARTUP: 3 frames
    // rapier +30
    {0, 12}, {0, 9}, {0, 9},

    // ACTIVE: 3 frames
    {0, 0}, {0, 0}, {0, 0},

    // RECOVERY: 6 frames
    // rapier -30
    {0, -4}, {0, -4}, {0, -4}, {0, -4}, {0, -4}, {0, -4}, {0, -4}, {0, -2}
};
constexpr uint8_t ATTACK_COUNT = sizeof ATTACK_ANIMATION / sizeof *ATTACK_ANIMATION;

#endif
