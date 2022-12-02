#ifndef STATES_HPP
#define STATES_HPP

#include <type_traits>

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

// USED FOR DEBUGGING
//#define DEBUG 1

using f16 = fpm::fixed_16_16;

enum struct CollisionState {
    NONE,
    WIN,
    LOSS,
};

enum struct Animation {
    NONE,
    ATTACK,
    LUNGE,
    PARRY,
    FEINT,
};

struct PlayerState {
    f16 pos;
    f16 sword;
    uint8_t anim_frame;
    Animation anim;
};

struct InputState {
    short direction;
    bool attack;
    bool lunge;
    bool parry;
    bool feint;
};

// The hash_combine used by BOOST
template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

struct GameState {
    uint frame;
    PlayerState p1;
    InputState i1;
    PlayerState p2;
    InputState i2;
    CollisionState col;
    bool opponent_input;
    size_t state_hash;

    static size_t genHash(const GameState& val) {
        std::size_t result = 0;
        hash_combine(result, val.frame);
        hash_combine(result, val.p1.pos.raw_value());
        hash_combine(result, val.p1.sword.raw_value());
        hash_combine(result, val.p1.anim_frame);
        hash_combine(result, static_cast<int>(val.p1.anim));
        hash_combine(result, val.i1.direction);
        hash_combine(result, val.i1.attack);
        hash_combine(result, val.i1.lunge);
        hash_combine(result, val.i1.parry);
        hash_combine(result, val.i1.feint);
        hash_combine(result, static_cast<int>(val.col));
        return result;
    }
};

const size_t PACKET_SIZE = 12;

struct NetworkState {
    InputState inputs;
    uint frame;
    size_t state_hash;
    bool valid = true;

    static void print(NetworkState& s){
        std::cout << "STATE:\n"
            << "\tInput: " << s.inputs.direction << " "
                           << s.inputs.attack << " "
                           << s.inputs.parry << " "
                           << s.inputs.feint << " "
                           << s.inputs.lunge << "\n"
            << "\tFrame: " << s.frame << "\n"
            << "\tValid: " << s.valid << "\n";
    }

    static std::array<char, PACKET_SIZE> serialize(NetworkState& s){
        int checksum = 0;
        std::array<char, PACKET_SIZE> buf;
        buf[0] = (s.state_hash >> 24) & 0xFF;
        buf[1] = (s.state_hash >> 16) & 0xFF;
        buf[2] = (s.state_hash >> 8) & 0xFF;
        buf[3] = s.state_hash & 0xFF;
        buf[4] = (s.frame >> 24) & 0xFF;
        buf[5] = (s.frame >> 16) & 0xFF;
        buf[6] = (s.frame >> 8) & 0xFF;
        buf[7] = s.frame & 0xFF;

        buf[8] = s.inputs.direction;
        buf[9] = 0x00
            | (s.inputs.attack << 3)
            | (s.inputs.parry << 2)
            | (s.inputs.feint << 1)
            | (s.inputs.lunge);

        for (int i=0; i <= 9; i++) {
            checksum += (unsigned char) buf[i];
        }
        buf[10] = (char) -(checksum % 256);
        return buf;
    }

    static NetworkState deserialize(char* buf) {
        int checksum = 0;
        NetworkState ret {};
        ret.state_hash = buf[3] & 0x000000FF
            | (buf[2] << 8)  & 0x0000FF00
            | (buf[1] << 16) & 0x00FF0000
            | (buf[0] << 24) & 0xFF000000;
        ret.frame = buf[7] & 0x000000FF
            | (buf[6] << 8)  & 0x0000FF00
            | (buf[5] << 16) & 0x00FF0000
            | (buf[4] << 24) & 0xFF000000;
        ret.inputs.direction = buf[8];
        ret.inputs.attack = (buf[9] >> 3) & 0x01;
        ret.inputs.parry  = (buf[9] >> 2) & 0x01;
        ret.inputs.feint  = (buf[9] >> 1) & 0x01;
        ret.inputs.lunge  =  buf[9]       & 0x01;

        for (int i=0; i <= 10; i++) {
            checksum += (unsigned char) buf[i];
        }
        ret.valid = (checksum % 256 == 0) && (checksum != 0);
        return ret;
    }
};

#endif // STATES_HPP
