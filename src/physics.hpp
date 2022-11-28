#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <deque>

#include <boost/circular_buffer.hpp>

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

#include "networking.hpp"
#include "collider.hpp"
#include "states.hpp"

#define ITER_ANIM(next, base, frame, TYPE) \
        (next).anim_frame = (frame);                                            \
        (next).pos = (base).pos + TYPE##_ANIMATION[(frame)].input_direction;    \
        (next).sword = (base).sword + TYPE##_ANIMATION[(frame)].sword_direction;

#define PROCESS_ANIM(next, base, TYPE) \
    case Animation::#TYPE:                                   \
        if ((base).anim_frame < TYPE##_COUNT) {              \
            ITER_ANIM(next, base, (base).anim_frame+1, TYPE) \
        } else {                                             \
            (next).anim_frame = 0;                           \
            (next).anim = Animation::NONE;                   \
        }                                                    \
        break;

using f16 = fpm::fixed_16_16;

const f16 WALK_SPEED = f16(0.75);
const size_t BUFFER = 10;

class Physics {
public:
    Physics();
    ~Physics();

    bool run(std::shared_ptr<Peer> _net);
    void abort();

    void update_inputs(const InputState& input);
    void update_state();

    PlayerState get(bool player);

private:
    void update();

    void buffer_push(GameState state);
    void process_input(PlayerState& next, const PlayerState& base, const InputState& input);

    std::atomic_uint frame_counter = 0;

    std::mutex _player_lock;
    Collider _p1_body, _p2_body;
    Collider _p1_rapier, _p2_rapier;

    std::deque<GameState> _rollback_buffer {};
    std::thread _physics_thread;
    bool _run_physics = true;

    std::mutex _input_lock;
    InputState _input {};

    std::shared_ptr<Peer> _networking;
};

#endif // PHYSICS
