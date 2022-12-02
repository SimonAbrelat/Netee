#ifndef NETWORKING_HPP
#define NETWORKING_HPP

#include <atomic>
#include <thread>
#include <deque>
#include <mutex>

#ifdef DEBUG
#include <chrono>
#include <cstdlib>
#endif

#include <enet/enet.h>

#include "states.hpp"

const int MAX_MSG_BUFFER = 30; // 2 seconds

class Peer {
public:
  Peer();
  ~Peer();

  virtual bool start() { return true; }
  virtual void sendState(NetworkState state) {};
  virtual void sendSync() {};
  virtual void sendWin(CollisionState c) {};
  virtual void sendRstComp() {};

  void stop();
  std::deque<NetworkState> readStates();
  bool newData();
  bool needSync();
  short needReset();

  void networkloop(ENetHost* sock);
#ifdef DEBUG
  void debugloop(ENetHost* sock);
#endif


protected:
  std::mutex opponent_lock;
  std::deque<NetworkState> opponent_states;
  std::atomic_bool new_states = { false };

  std::atomic_bool need_sync = { false };
  std::atomic_short need_reset = { 0 }; // 0 = no, 1 = yes and I won, 2 = yes and I lost

  std::atomic_bool _is_terminated = { false };
  std::thread _recv_thread;

#ifdef DEBUG
  std::thread _debug_thread;
  std::atomic_bool _is_debug = { true };

  struct DebugMsg {
    NetworkState msg;
    int time;
  };

  std::mutex msg_lock;
  std::deque<DebugMsg> msg_queue;

#endif
};

class Server : public Peer {
public:
  Server(uint port)
    : Peer(), _port(port)
  {};

  ~Server();

  bool start() override;
  void sendState(NetworkState state) override;
  void sendSync() override;
  void sendWin(CollisionState c) override;

private:
  uint _port;

  ENetHost* server;
  ENetPeer* client;
};

class Client : public Peer {
public:
  Client(const char* addr, uint port)
    : Peer(), _host(addr), _port(port)
  {};

  ~Client();


  bool start() override;
  void sendState(NetworkState state) override;
  void sendSync() override;
  void sendWin(CollisionState c) override;

private:
  const char * _host;
  uint _port;

  ENetHost* client;
  ENetPeer* server;
};

#endif // NETWORKING
