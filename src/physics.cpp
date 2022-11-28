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
    _input_lock.lock();
    _input = input;
    _input_lock.unlock();
}

PlayerState Physics::get(bool player){
    _player_lock.lock();
    // Most recent completed frame
    GameState current = _rollback_buffer.front();
    // Get the player, information
    PlayerState ret {(player) ? current.p1 : current.p2};
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
        const auto next_cycle = std::chrono::steady_clock::now() + 33ms;

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
                if (it->frame < oldest_frame && oldest_frame != 0) {
                    oldest_frame = it->frame;
                }
                if (it->frame > newest_input.frame) {
                    newest_input = *it;
                }
            }
            //std::cout << "Curr Frame: " << frame_counter << ", Oldest frame: " << oldest_frame << '\n';
        }

        _player_lock.lock();
        // Calculate Rollbacks
        if (new_input) {
            // How deep into the buffer we go
            uint i = frame_counter - oldest_frame;
            if (i >= _rollback_buffer.size()) {
                std::cout << "ERROR: BUFFER NOT LONG ENOUGH: " << oldest_frame << "\n";
            } else {
                if (oldest_frame != frame_counter) {
                    // Rollback code
                    for (auto rol = _rollback_buffer.rend() - i; rol != _rollback_buffer.rend(); ++rol) {
                        if (rol->opponent_input == false) {
                            for (auto in = opp_inputs.cbegin(); in != opp_inputs.cend(); ++in) {
                                // Add missing opponent input
                                if (in->frame == rol->frame) {
                                    rol->i2 = in->inputs;
                                    rol->opponent_input = true;
                                    //std::cout << "UPDATING FRAME: " << rol->frame << '\n';
                                }
                            }
                        }
                        auto base = rol - 1; // The frame before the current frame
                        rol->p2.pos = base->p2.pos + (WALK_SPEED * rol->i2.direction);
                    }
                }
            }
        }

        // Update current iteration
        _p1_body.x += WALK_SPEED * curr.direction;
        _p2_body.x += WALK_SPEED * newest_input.inputs.direction;
        std::clog << "P1: "  << (int)_p1_body.x << ", P2: " << (int)_p2_body.x << '\n';

        std::cout << "Rollback state: ";
        for (auto it = _rollback_buffer.cbegin(); it != _rollback_buffer.cend(); ++it) {
            std::cout << it->opponent_input << " ";
        }
        std::cout << "\n";

        buffer_push(GameState{
            frame_counter,
            PlayerState {_p1_body.x, 0, Animation::NONE}, curr,
            PlayerState {_p2_body.x, 0, Animation::NONE}, newest_input.inputs,
            newest_input.frame == frame_counter
        });
        _player_lock.unlock();

        _networking->sendState(NetworkState{curr, frame_counter, 0});
        std::this_thread::sleep_until(next_cycle);
    }
    std::terminate();
}
