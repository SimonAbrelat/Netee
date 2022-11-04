#include "physics.hpp"

Physics::Physics() {
    _p1_body = Collider(100,100,10,10);
    _p2_body = Collider(100,100,10,10);
};

Physics::~Physics() {
    _physics_thread.join();
};

bool Physics::run(std::shared_ptr<Peer> net){
    _networking = net;
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
    _networking->sendState(NetworkState{input, frame_counter, 0});
    _input_lock.lock();
    _input = input;
    _input_lock.unlock();
}

PlayerState Physics::get(bool player){
    _player_lock.lock();
    PlayerState ret {((player) ? _p1_body : _p2_body).x, f16(0)};
    _player_lock.unlock();
    return ret;
};

void Physics::buffer_push(GameState state) {
    if (_rollback_buffer.size() == BUFFER) {
        _rollback_buffer.pop_back();
    }
    _rollback_buffer.push_front(state);
}

void Physics::update() {
    using std::chrono::operator""ms;
    int count = 0;
    NetworkState opp_input;
    while (_run_physics) {
        frame_counter++;
        const auto next_cycle = std::chrono::steady_clock::now() + 17ms;
        _input_lock.lock();
        InputState curr = _input;
        _input_lock.unlock();


        bool new_input = _networking->newData();
        if (new_input) {
            opp_input = _networking->readState();
        }

        _player_lock.lock();
        _p1_body.x += WALK_SPEED * curr.direction;
        long i = frame_counter - opp_input.frame;
        if (new_input && i > 0 && i < 10) {
            for(size_t j = i; j > 0; j--) {
                if (_rollback_buffer.at(i).opponent_input == false) {
                    _rollback_buffer.at(i).i2 = opp_input.inputs;
                }
                int diff = opp_input.inputs.direction != _rollback_buffer.at(i).i2.direction;
                _rollback_buffer.at(i).p2.pos += WALK_SPEED * diff;
            }
            _rollback_buffer.at(i).opponent_input = true;
            _p2_body.x = _rollback_buffer.at(0).p2.pos + (WALK_SPEED * opp_input.inputs.direction);
        } else {
            _p2_body.x += WALK_SPEED * opp_input.inputs.direction;
        }
        std::cout << "P1 : " << (int) _p1_body.x << ", P2 : " << (int) _p2_body.x << '\n';
        _player_lock.unlock();

        buffer_push(GameState{
            frame_counter,
            PlayerState {_p1_body.x, f16(0)}, curr,
            PlayerState {_p2_body.x, f16(0)}, opp_input.inputs,
            new_input
        });

        std::this_thread::sleep_until(next_cycle);
    }
    std::terminate();
}
