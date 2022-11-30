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
    CollisionState getWin();

private:
    void update();

    void buffer_push(GameState state);
    void process_input(PlayerState& next, const PlayerState& base, const InputState& input, bool mirror = false);
    CollisionState process_collisions(PlayerState& p1, PlayerState& p2);

    std::atomic_uint frame_counter = { 0 } ;

    std::mutex _player_lock;
    Collider _p1_body, _p2_body;
    Collider _p1_rapier, _p2_rapier;

    std::deque<GameState> _rollback_buffer;
    std::thread _physics_thread;
    std::atomic_bool _run_physics = true;

    std::mutex _input_lock;
    InputState _input {};

    std::mutex _game_lock;
    CollisionState _win = CollisionState::NONE;

    std::shared_ptr<Peer> _networking;
};

#endif // PHYSICS
