#ifndef MOVES_HPP
#define MOVES_HPP

// TODO: Should these be f16s???
struct Move {
    short input_direction;
    short sword_direction;
    bool is_active = false;
};

const Move ATTACK_ANIMATION[] {
    // STARTUP: 3 frames
    // rapier +30
    {0, 12, true}, {0, 9, true}, {0, 9, true},

    // ACTIVE: 3 frames
    {0, 0, true}, {0, 0, true}, {0, 0, true},

    // RECOVERY: 6 frames
    // rapier -30
    {0, -5}, {0, -5}, {0, -5}, {0, -5}, {0, -5}, {0, -5},

    // END LAG: 2 frames
    {0, 0}, {0, 0}
};
constexpr uint8_t ATTACK_COUNT = sizeof ATTACK_ANIMATION / sizeof *ATTACK_ANIMATION;

const Move LUNGE_ANIMATION[] {
    // STARTUP: 7 frames
    // input +30
    // rapier +60
    {5, 6, true}, {5, 5, true}, {5, 15, true}, {5, 9, true}, {5, 9, true}, {4, 8, true}, {1, 8, true},

    // ACTIVE: 4 frames
    {0, 0, true}, {0, 0, true}, {0, 0, true}, {0, 0, true},

    // RECOVERY: 10 frames
    // rapier -30
    {0, -3}, {0, -3}, {0, -3}, {0, -3}, {0, -4}, {0, -4}, {0, -4}, {0, -2}, {0, -2}, {0, -2},

    // END LAG: 3 frames
    {0, 0}, {0, 0}, {0, 0}
};
constexpr uint8_t LUNGE_COUNT = sizeof LUNGE_ANIMATION / sizeof *LUNGE_ANIMATION;

const Move PARRY_ANIMATION[] {
    // STARTUP: 3 frames
    // rapier +15
    {0, 10}, {0, 3}, {0, 2},

    // ACTIVE: 10 frames
    {0, 0, true}, {0, 0, true}, {0, 0, true}, {0, 0, true}, {0, 0, true}, {0, 0, true}, {0, 0, true}, {0, 0, true}, {0, 0, true}, {0, 0, true},

    // RECOVERY: 6 frames
    // rapier -15
    {0, -2}, {0, -2}, {0, -2}, {0, -3}, {0, -3}, {0, -3},

    // END LAG: 2 frames
    {0, 0}, {0, 0}
};
constexpr uint8_t PARRY_COUNT = sizeof PARRY_ANIMATION / sizeof *PARRY_ANIMATION;

const Move FEINT_ANIMATION[] {
    // STARTUP: 7 frames
    // input +30
    // rapier +60
    {5, 6}, {5, 5}, {5, 15}, {5, 9}, {5, 9}, {4, 8}, {1, 8},

    // RECOVERY: 5 frames
    // rapier -30
    {0, -6}, {0, -6}, {0, -6}, {0, -6}, {0, -6},

    // END LAG: 3 frames
    {0, 0}, {0, 0}, {0, 0}
};
constexpr uint8_t FEINT_COUNT = sizeof FEINT_ANIMATION / sizeof *FEINT_ANIMATION;
#endif
