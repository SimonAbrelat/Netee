#include "physics.hpp"
#include "moves.hpp"

#include <iostream>

Physics::Physics() {
    _p1_body = Collider(100,100,50,50);
    _p2_body = Collider(530,100,50,50);
    _p1_rapier = Collider(100,100,10,100);
    _p2_rapier = Collider(480,100,10,100);
};

Physics::~Physics() {
    _physics_thread.join();
};

bool Physics::run() {
    try {
        _physics_thread = std::thread(&Physics::update, this);
    } catch(std::exception e) {
        return false;
    }
    return true;
}

void Physics::abort() {
    _run_physics = false;
    _physics_thread.join();
}

void Physics::update_inputs(const InputState& input) {
    _input_lock.lock();
    _input = input;
    _input_lock.unlock();
}

PlayerState Physics::get(bool player){
    _player_lock.lock();
    PlayerState ret {((player) ? _p1_body : _p2_body).x, ((player) ? _p1_rapier : _p2_rapier).x};
    _player_lock.unlock();
    return ret;
};

void Physics::update() {
    using std::chrono::operator""ms;
    int count = 0;

    int attack_count = ATTACK_COUNT;

    while (_run_physics) {
        const auto next_cycle = std::chrono::steady_clock::now() + 17ms;
        _input_lock.lock();
        InputState curr = _input;
        _input_lock.unlock();

        _player_lock.lock();
        _p1_body.x += WALK_SPEED * curr.direction;
        _p1_rapier.x += WALK_SPEED * curr.direction;

        // attack
        if (curr.attack && attack_count == ATTACK_COUNT) attack_count = 0;
        if (attack_count < ATTACK_COUNT) {
            _p1_rapier.x += attack[attack_count].rapier_direction;
            attack_count++;
        }

        //std::cout << "P1 x: " << (int) _p1_body.x << " is colliding: " << _p1_body.is_colliding(_p2_body) << '\n';
        _player_lock.unlock();

        std::this_thread::sleep_until(next_cycle);
    }
    std::terminate();
}
