#include "physics.hpp"

#include <iostream>

Physics::Physics() {
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

bool Physics::abort() {
    _physics_thread.join();
}

void Physics::update_inputs(InputState input) {
    _input_lock.lock();
    _input = input;
    _input_lock.unlock();
}

void Physics::update() {
    using std::chrono::operator""ms;
    int count = 0;
    while (true) {
        const auto next_cycle = std::chrono::steady_clock::now() + 17ms;
        _input_lock.lock();
        InputState curr = _input;
        _input_lock.unlock();

        std::cout << "PHYSICS DIRECTION: " << curr.direction << '\n';

        std::this_thread::sleep_until(next_cycle);
    }
}
