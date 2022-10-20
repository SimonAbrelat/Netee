#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <chrono>
#include <thread>
#include <mutex>

#include <boost/circular_buffer.hpp>

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

#include "collider.hpp"
#include "states.hpp"

using f16 = fpm::fixed_16_16;

class Physics {
public:
    Physics();
    ~Physics();

    bool run();
    bool abort();

    void update_inputs(InputState input);
    void update_state();

private:
    void update();

    Collider* _p1_body, _p1_sword, _p2_body, _p2_sword;

    std::mutex _buffer_lock;
    boost::circular_buffer<GameState> _rollback_buffer;
    std::thread _physics_thread;

    std::mutex _input_lock;
    InputState _input {};
};

#endif // PHYSICS
