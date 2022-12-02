#ifndef STATES_HPP
#define STATES_HPP

#include <fpm/fixed.hpp>
#include <fpm/math.hpp>

#include <string>
// USED FOR DEBUGGING
//#define DEBUG 1
#define LOG 1

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
    bool anim_active;
    bool is_clank;
};

struct InputState {
    short direction;
    bool attack;
    bool lunge;
    bool parry;
    bool feint;
};

struct GameState {
    uint frame;
    PlayerState p1;
    InputState i1;
    PlayerState p2;
    InputState i2;
    CollisionState col;
    bool opponent_input;
};

const size_t PACKET_SIZE = 12;

struct NetworkState {
    InputState inputs;
    uint frame;
    uint state_hash;
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

enum struct PacketType {
    SYNC = 0,
    WIN = 1 << 0,
    LOSS = 1 << 2,
};

const size_t METADATA_SIZE = 6;

struct MetaData {
    PacketType type;
    uint frame;
    bool valid = true;

    static std::array<char, METADATA_SIZE> serialize(const MetaData& d){
        int checksum = 0;
        std::array<char, METADATA_SIZE> buf;
        buf[0] = static_cast<char>(d.type);
        buf[1] = (d.frame >> 24) & 0xFF;
        buf[2] = (d.frame >> 16) & 0xFF;
        buf[3] = (d.frame >> 8) & 0xFF;
        buf[4] = d.frame & 0xFF;

        for (int i=0; i < METADATA_SIZE-1; i++) {
            checksum += (unsigned char) buf[i];
        }
        buf[5] = (char) -(checksum % 256);
        return buf;
    }

    static MetaData deserialize(char* buf) {
        int checksum = 0;
        MetaData ret {};
        ret.type = static_cast<PacketType>(buf[0]);
        ret.frame = buf[4] & 0x000000FF
            | (buf[3] << 8)  & 0x0000FF00
            | (buf[2] << 16) & 0x00FF0000
            | (buf[1] << 24) & 0xFF000000;

        for (int i=0; i < METADATA_SIZE; i++) {
            checksum += (unsigned char) buf[i];
        }
        ret.valid = (checksum % 256 == 0) && (checksum != 0);
        return ret;
    }
};

#endif // STATES_HPP
