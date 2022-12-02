#include "physics.hpp"
#include "moves.hpp"

Physics::Physics() {
    reset();
};

Physics::~Physics() {
    _run_physics = false;
    _physics_thread.join();
};

void Physics::reset() {
    _player_lock.lock();
    _p1_body = Collider(100,100,50,50);
    _p2_body = Collider(530,100,50,50);
    _p1_rapier = Collider(100,100,10,100);
    _p2_rapier = Collider(480,100,10,100);
    for (int i = 0; i < BUFFER; i++) {
        buffer_push(GameState{
            0,
            PlayerState {_p1_body.x, _p1_rapier.x, 0, Animation::NONE, false, false}, InputState{},
            PlayerState {_p2_body.x, _p2_rapier.x, 0, Animation::NONE, false, false}, InputState{},
            CollisionState::NONE, true
        });
    }
    _player_lock.unlock();
}

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
    GameState current = _rollback_buffer.at(2);
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

// Updates the state of next based on base and the current frame of the animation
// This must be a macro since the variables referenced change based on TYPE
#define ITER_ANIM(next, base, frame, TYPE, mirror) \
        (next).anim = Animation::TYPE;                                                                  \
        (next).anim_frame = (frame);                                                                    \
        (next).anim_active = TYPE##_ANIMATION[(frame)]. is_active;                                      \
        (next).pos = (base).pos + (((mirror) ? -1 : 1) * TYPE##_ANIMATION[(frame)].input_direction);    \
        (next).sword = (base).sword + (((mirror) ? -1 : 1) * TYPE##_ANIMATION[(frame)].sword_direction);

// Manages the lifetime of animations and resets their state when complete
// This must be a macro since the variables referenced change based on TYPE
#define PROCESS_ANIM(next, base, mirror, TYPE) \
    case Animation::TYPE:                                            \
        if ((base).anim_frame < TYPE##_COUNT-1) {                      \
            ITER_ANIM(next, base, (base).anim_frame+1, TYPE, mirror) \
        } else {                                                     \
            (next).anim_frame = 0;                                   \
            (next).anim = Animation::NONE;                           \
        }                                                            \
        break;

// Updates the Player state based on their current input and their prior state.
// Uses macros to start and manage animation lifetimes. Mirror acts for the opponent
// so that both players see their player as the right.
void Physics::process_input(PlayerState& next, const PlayerState& base, const InputState& input, bool mirror) {
    // Handle each animation state
    switch (base.anim) {
        // All actions have the same logic, but different variables
        PROCESS_ANIM(next, base, mirror, ATTACK)
        PROCESS_ANIM(next, base, mirror, LUNGE)
        PROCESS_ANIM(next, base, mirror, PARRY)
        PROCESS_ANIM(next, base, mirror, FEINT)
        // None is the case where new actions can start or they can move
        case Animation::NONE:
            // Start new actions
            if (input.attack) {
               ITER_ANIM(next, base, 0, ATTACK, mirror);
            } else if(input.lunge) {
               ITER_ANIM(next, base, 0, LUNGE, mirror);
            } else if(input.parry) {
               ITER_ANIM(next, base, 0, PARRY, mirror);
            } else if(input.feint) {
               ITER_ANIM(next, base, 0, FEINT, mirror);
            } else {
                // Smooth player movement
                next.pos = base.pos + ((mirror ? -1 : 1) * (WALK_SPEED * input.direction));
                next.sword = base.sword + ((mirror ? -1 : 1) * (WALK_SPEED * input.direction));
            }
            break;
    }
}

// Checks collisions to figure out which player wins
CollisionState Physics::process_collisions(PlayerState& p1, PlayerState& p2) {
    // Update the colliders
    _p1_body.x = p1.pos;
    _p1_rapier.x = p1.sword;
    _p2_body.x = p2.pos;
    _p2_rapier.x = p2.sword;

    // Reset hitbox's activity
    if (p1.anim == Animation::NONE) {
        _p1_rapier.is_active = false;
        p1.anim_active = false;
        p1.is_clank = false;
    } else {
        _p1_rapier.is_active = p1.anim_active && !p1.is_clank;
    }

    if (p2.anim == Animation::NONE) {
        _p2_rapier.is_active = false;
        p2.anim_active = false;
        p2.is_clank = false;
    } else {
        _p2_rapier.is_active = p2.anim_active && !p2.is_clank;
    }

    // Calculates the collision
    bool win = false;
    bool loss = false;

    bool clank = false;
    if (p1.anim == Animation::ATTACK || p1.anim == Animation::LUNGE) {
        win = Collider::is_colliding(_p1_rapier, _p2_body);
        bool swords = Collider::is_colliding(_p1_rapier, _p2_rapier);
        if (swords && (p2.anim == Animation::ATTACK || p2.anim == Animation::LUNGE)) {
            _p1_rapier.is_active = _p2_rapier.is_active = false;
            p1.is_clank = p2.is_clank = true;
        }
        if (swords && (p2.anim == Animation::PARRY)) {
            _p1_rapier.is_active = false;
            p1.is_clank = true;
            p2.anim = Animation::NONE;
            p2.anim_frame = 0;
        }
    }
    if (p2.anim == Animation::ATTACK || p2.anim == Animation::LUNGE) {
        loss = Collider::is_colliding(_p2_rapier, _p1_body);
        bool swords = Collider::is_colliding(_p1_rapier, _p2_rapier);
        if (swords && (p1.anim == Animation::ATTACK || p1.anim == Animation::LUNGE)) {
            _p1_rapier.is_active = _p2_rapier.is_active = false;
            p1.is_clank = p2.is_clank = true;
        }
        if (swords && (p1.anim == Animation::PARRY)) {
            _p2_rapier.is_active = false;
            p2.is_clank = true;
            p1.anim = Animation::NONE;
            p1.anim_frame = 0;
        }
    }

    // Returns state
    if (win && loss) {
       p1.is_clank = true;
       p2.is_clank = true;
    } else if (win) {
        return CollisionState::WIN;
    } else if (loss) {
        return CollisionState::LOSS;
    }
    return CollisionState::NONE;
}

void Physics::update() {
    // Makes the physics time calcs easier
    using std::chrono::operator""ms;

    // Outside of the loop in case the network does not have inputs
    std::deque<NetworkState> opp_inputs;

    // Check if two active hitboxes are touching
    bool clank = false;
#ifdef LOG
    auto prev_cycle =  std::chrono::steady_clock::now();
#endif

    while (_run_physics) {
        auto next_cycle = std::chrono::steady_clock::now() + 16ms; // 60 fps
        //auto next_cycle = std::chrono::steady_clock::now() + 32ms; // 30 fps
        //auto next_cycle = std::chrono::steady_clock::now() + 100ms; // 10 fps
        if (_networking->needSync()) {
            std::clog << "SYNCING\n";
            std::this_thread::sleep_until(next_cycle);
            continue;
        }
        short reset_result = _networking->needReset();
        if (reset_result) {
            std::clog << ((reset_result == 1) ? "YOU WIN\n" : "YOU LOSE\n");
            reset();
            std::this_thread::sleep_until(next_cycle);
            continue;
        }

        // Increment frame
        frame_counter++;

#ifdef LOG
        std::clog <<
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - prev_cycle).count()
            << "\n";
        prev_cycle = std::chrono::steady_clock::now();
#endif

        // Get new local inputs
        _input_lock.lock();
        InputState curr = _input;
        _input_lock.unlock();

        // Get opponent inputs
        bool new_input = _networking->newData();
        uint oldest_frame = std::numeric_limits<unsigned int>::max(); // Determines rollback distance
        NetworkState newest_input; // Used for the speculative input on the current frame
        if (new_input) {
            opp_inputs = _networking->readStates();
            newest_input = opp_inputs.front();
            // Iterates through everything in the buffer returned from the network
            for (auto it = opp_inputs.cbegin(); it != opp_inputs.cend(); ++it) {
                if (it->frame < oldest_frame && oldest_frame > 0) {
                    oldest_frame = it->frame;
                }
                if (it->frame > newest_input.frame) {
                    newest_input = *it;
                }
            }
        }
        _player_lock.lock();
        // Calculate Rollbacks
        if (new_input) {
            // How deep into the buffer we go
            uint i = frame_counter - oldest_frame;
            if (i >= _rollback_buffer.size()) {
                std::cout << "ERROR: BUFFER NOT LONG ENOUGH: " << oldest_frame << "\n";
                _networking->sendSync();
            } else {
                if (oldest_frame != frame_counter) {
                    // Rollback code
                    // Explanation:
                    //   To rollback we need to resimulate every frame after the new input.
                    //   So, we use a reverse iterator because it's more efficient. rend()
                    //   is the first element, so we go i elements deeper into the array.
                    for (auto rol = _rollback_buffer.rend() - i; rol != _rollback_buffer.rend(); ++rol) {
                        if (rol->opponent_input == false) {
                            // If input is missing check all opponent inputs
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
                        process_input(rol->p2, base->p2, rol->i2, true);
                        rol->col = process_collisions(rol->p1, rol->p2);
                    }
                }
            }
        }

        // Initialized the player states
        PlayerState player1 = _rollback_buffer.front().p1;
        PlayerState player2 = _rollback_buffer.front().p2;

        // Updates the player states based on their inputs
        process_input(player1, _rollback_buffer.front().p1, curr, false);
        process_input(player2, _rollback_buffer.front().p2, newest_input.inputs, true);
        CollisionState collision = process_collisions(player1, player2);

        // Logs the state of the physics and rollback buffer
        //std::clog << "P1: "  << (int)player1.pos << ", P2: " << (int)player2.pos << '\n';
#ifdef LOG
        std::clog << "Frame " << frame_counter << " Rollback state: ";
        for (auto it = _rollback_buffer.cbegin(); it != _rollback_buffer.cend(); ++it) {
            std::clog << it->opponent_input << " ";
        }
        std::clog << "\n";
#endif
        /*
        std::clog << "Frame " << frame_counter << " missing: ";
        for (auto it = _rollback_buffer.cbegin(); it != _rollback_buffer.cend(); ++it) {
            if (!it->opponent_input) {
                std::clog << it->frame << " ";
            }
        }
        std::clog << "\n";
        */

        // Push the current state into the rollback buffer
        buffer_push(GameState{
            frame_counter,
            player1, curr,
            player2, newest_input.inputs,
            collision,
            newest_input.frame == frame_counter
        });
        _player_lock.unlock();

        // Send the locked input to the opponent
        _networking->sendState(NetworkState{curr, frame_counter, 0});

        // Resolves agreed upon collisions
        bool has_consensus = true;
        for (auto rol = _rollback_buffer.rbegin(); rol != _rollback_buffer.rend(); ++rol) {
            has_consensus &= rol->opponent_input;
            if (has_consensus) {
                if (rol->col == CollisionState::WIN
                 || rol->col == CollisionState::LOSS) {
                    _networking->sendWin(rol->col);
                    break;
                }
            }
        }

        // Waits so the loop happens in 1/60th of a second
        std::this_thread::sleep_until(next_cycle);
    }
    std::quick_exit(0);
}
