#ifndef NETWORKING_HPP
#define NETWORKING_HPP

#include <atomic>
#include <thread>
#include <queue>
#include <deque>
#include <mutex>

#include <enet/enet.h>

#include "states.hpp"

const int MAX_MSG_BUFFER = 120; // 2 seconds

class Peer {
public:
  Peer();
  ~Peer();

  virtual bool start() { return true; }
  virtual void sendState(NetworkState state) {};

  void stop();
  std::deque<NetworkState> readStates();
  bool newData();

  void networkloop(ENetHost* sock);

protected:
  std::mutex opponent_lock;
  std::deque<NetworkState> opponent_states;
  std::atomic_bool new_states = false;

  std::atomic_bool _is_terminated = false;
  std::thread _recv_thread;
};

class Server : public Peer {
public:
  Server(uint port)
    : Peer(), _port(port)
  {};

  ~Server();

  bool start() override;
  void sendState(NetworkState state) override;

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

private:
  const char * _host;
  uint _port;

  ENetHost* client;
  ENetPeer* server;
};

#endif // NETWORKING
