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
    if (_rollback_buffer.size() == BUFFER + 1) {
        _rollback_buffer.pop_back();
    }
    _rollback_buffer.push_front(state);
}

void Physics::update() {
    using std::chrono::operator""ms;
    int count = 0;
    std::deque<NetworkState> opp_inputs;
    while (_run_physics) {
        // Setup physics loop
        frame_counter++;
        const auto next_cycle = std::chrono::steady_clock::now() + 17ms;

        // Get new local inputs
        _input_lock.lock();
        InputState curr = _input;
        _input_lock.unlock();

        // Get opponent inputs
        bool new_input = _networking->newData();
        uint oldest_frame = std::numeric_limits<unsigned int>::max();
        NetworkState newest_input;
        if (new_input) {
            opp_inputs = _networking->readStates();
            newest_input = opp_inputs.front();
            for (auto it = opp_inputs.cbegin(); it != opp_inputs.cend(); ++it) {
                if (it->frame < oldest_frame) {
                    oldest_frame = it->frame;
                }
                if (it->frame > newest_input.frame) {
                    newest_input = *it;
                }
            }
            std::cout << "Oldest frame: " << oldest_frame << '\n';
        }

        /*
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
        */
        _player_lock.lock();
            // Calculate Rollbacks
            if (new_input) {
            uint i = (frame_counter - 1) - oldest_frame; // How deep into the buffer we go
            if (i > BUFFER) {
                std::cout << "ERROR: BUFFER NOT LONG ENOUGH\n";
            }
            //std::cout << "GO BACK: " << i << " TO FRAME " << oldest_frame << "\n";
            //std::cout << "ITERATING: ";
            if (oldest_frame != frame_counter) {
                // Rollback code
                for (size_t j = i; j > 0; j--) {
                    // Update opponent_input and iterate all states from then on
                    //std::cout << j  << " ";
                        /*
                    if (_rollback_buffer.at(j).opponent_input == false) {
                        std::cout << "UPDATING FRAME: " << _rollback_buffer.at(j).frame << '\n';
                        // Check all opponent_inputs
                        for (auto it = opp_inputs.cbegin(); it != opp_inputs.cend(); ++it) {
                            // Add missing opponent input
                            if (it->frame == _rollback_buffer.at(j).frame) {
                                _rollback_buffer.at(j).i2 = it->inputs;
                                _rollback_buffer.at(j).opponent_input = true;
                            }
                        }
                    }
                        */
                    // Update opponent Movement

                        /*
                    _rollback_buffer.at(j).p2.pos =
                        ((j+1 >= _rollback_buffer.size()) ? f16(0) : _rollback_buffer.at(j+1).p2.pos)
                        + (WALK_SPEED * _rollback_buffer.at(j).i2.direction);
                        */
                }
            }
            //std::cout << " \n";
        }

        // Update current iteration
        _p1_body.x += WALK_SPEED * curr.direction;
        _p2_body.x += WALK_SPEED * newest_input.inputs.direction;
        _player_lock.unlock();

        buffer_push(GameState{
            frame_counter,
            PlayerState {_p1_body.x, f16(0)}, curr,
            PlayerState {_p2_body.x, f16(0)}, newest_input.inputs,
            newest_input.frame == frame_counter
        });

        std::cout << "Opponent state: ";
        for (auto it = opp_inputs.cbegin(); it != opp_inputs.cend(); ++it) {
            std::cout << it->frame << " ";
        }
        std::cout << "\n";

        std::this_thread::sleep_until(next_cycle);
    }
    std::terminate();
}
