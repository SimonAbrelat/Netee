#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>

#include <boost/circular_buffer.hpp>

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

#include "networking.hpp"
#include "collider.hpp"
#include "states.hpp"


using f16 = fpm::fixed_16_16;

const f16 WALK_SPEED = f16(0.5);

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


    std::atomic_long frame_counter = 0;

    std::mutex _player_lock;
    Collider _p1_body, _p2_body;

    std::mutex _buffer_lock;
    boost::circular_buffer<GameState> _rollback_buffer;
    std::thread _physics_thread;
    bool _run_physics = true;

    std::mutex _input_lock;
    InputState _input {};

    std::shared_ptr<Peer> _networking;
};

#endif // PHYSICS
