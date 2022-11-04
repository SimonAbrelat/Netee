#ifndef NETWORKING_HPP
#define NETWORKING_HPP

#include <atomic>
#include <thread>
#include <queue>
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
  NetworkState readState();
  bool newData();

  void recvloop(UDTSOCKET recver);
protected:


  std::mutex opponent_lock;
  NetworkState opponent_state;
  std::atomic_bool new_opponent_state = false;

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
  UDTSOCKET sock;
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
  UDTSOCKET sock;

  const char * _host;
  const char * _port;
};

#endif // NETWORKING
