#ifndef NETWORKING_HPP
#define NETWORKING_HPP

#include <atomic>
#include <thread>
#include <queue>
#include <deque>
#include <mutex>

#include <udt.h>

#include "states.hpp"

const int MAX_MSG_BUFFER = 120; // 2 seconds

class Peer {
public:
  Peer();
  ~Peer();

  virtual bool start() { return true; }
  virtual bool sendState(NetworkState state) { return false; }

  void stop();
  std::deque<NetworkState> readStates();
  bool newData();

  void recvloop(UDTSOCKET recver);
protected:


  std::mutex opponent_lock;
  std::deque<NetworkState> opponent_states;
  std::atomic_bool new_states = false;

  std::mutex msg_lock;
  std::queue<NetworkState> msg_queue;

  std::atomic_bool _is_terminated = false;
  std::thread _recv_thread;

};

class Server : public Peer {
public:
  Server(const char* port)
    : Peer(), _port(port)
  {};

  ~Server();

  bool start();
  bool sendState(NetworkState state);

private:
  const char* _port;
  UDTSOCKET recv;
  UDTSOCKET send;
  UDTSOCKET serv;
};

class Client : public Peer {
public:
  Client(const char* addr, const char* port)
    : Peer(), _host(addr), _port(port)
  {};

  ~Client();


  bool start();
  bool sendState(NetworkState state);

private:
  UDTSOCKET recv;
  UDTSOCKET send;

  const char * _host;
  const char * _port;
};

#endif // NETWORKING
