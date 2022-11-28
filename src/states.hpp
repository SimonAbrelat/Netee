#ifndef STATES_HPP
#define STATES_HPP

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

using f16 = fpm::fixed_16_16;

struct PlayerState {
    f16 pos;
    f16 r_pos;
};

struct GameState {
    long frame;
    PlayerState p1;
    PlayerState p2;
    bool opponent_input;
};

struct InputState {
    short direction;
    bool attack;
    bool parry;
    bool feint;
    bool lunge;
};

struct NetworkState {
    InputState inputs;
    long frame;
    long state_hash;

    static std::array<char, 9> serialize(NetworkState& s){
        std::array<char, 9> buf;
        buf[0] =  s.state_hash        & 0xFF;
        buf[1] = (s.state_hash >>  8) & 0xFF;
        buf[2] = (s.state_hash >> 16) & 0xFF;
        buf[3] = (s.state_hash >> 24) & 0xFF;
        buf[4] =  s.frame        & 0xFF;
        buf[5] = (s.frame >>  8) & 0xFF;
        buf[6] = (s.frame >> 16) & 0xFF;
        buf[7] = (s.frame >> 24) & 0xFF;
        buf[8] = s.inputs.direction;
        buf[9] = 0x00
            | (s.inputs.attack << 3)
            | (s.inputs.parry << 2)
            | (s.inputs.feint << 1)
            | (s.inputs.lunge);
        return buf;
    }

    static NetworkState deserialize(char* buf) {
        NetworkState ret {};
        ret.state_hash = buf[0] | (buf[1] << 8L) | (buf[2] << 16L) | (buf[3] << 24L);
        ret.frame      = buf[4] | (buf[5] << 8L) | (buf[6] << 16L) | (buf[7] << 24L);
        ret.inputs.direction = buf[8];
        ret.inputs.attack = (buf[9] >> 3) & 0x01;
        ret.inputs.parry  = (buf[9] >> 2) & 0x01;
        ret.inputs.feint  = (buf[9] >> 1) & 0x01;
        ret.inputs.lunge  =  buf[9]       & 0x01;
        return ret;
    }
};

#endif // STATES_HPP
