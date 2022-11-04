#ifndef NETWORKING_HPP
#define NETWORKING_HPP

#include <exception>
#include <chrono>
#include <thread>
#include <queue>
#include <mutex>

#include <dyad/dyad.h>

#include "states.hpp"

const int MAX_MSG_BUFFER = 120; // 2 seconds

class Peer {
public:
  Peer(int port) {
    dyad_init();
    _socket = dyad_newStream();
  }

  ~Peer() {
    stop();
    dyad_shutdown();
  }

  bool start() {
    try {
        _network_thread = std::thread(&Peer::update, this);
    } catch(std::exception e) {
        return false;
    }
    return true;
  }

  void stop() {
    _is_terminated = true;
    if (_network_thread.joinable()) {
      _network_thread.join();
    }
  }

  void sendState(NetworkState& state) {
    msg_lock.lock();
    if (msg_queue.size() == MAX_MSG_BUFFER) {
      msg_queue.pop();
    }
    msg_queue.push(state);
    msg_lock.unlock();
  }

  NetworkState readState() {
    opponent_lock.lock();
    NetworkState ret = opponent_state;
    new_opponent_state = false;
    opponent_lock.unlock();
    return ret;
  }

  bool newData() {
    opponent_lock.lock();
    bool ret = new_opponent_state;
    opponent_lock.unlock();
    return ret;
  }

protected:
  void update() {
    while (dyad_getStreamCount() > 0 && !_is_terminated) {
      dyad_update();
    }
    if (_is_terminated) {
      std::terminate();
    }
  }

  static void onData(dyad_Event *e) {
    /*
    opponent_lock.lock();
    opponent_state = NetworkState::deserialize(e->data);
    new_opponent_state = true;
    opponent_lock.lock();
    */
  }

  static void sendMessage(dyad_Event *e) {
    /*
    msg_queue.lock();
    if (!msg_queue.empty()) {
      dyad_writef(e->stream, NetworkState::serialize(msg_queue.front());
    }
    msg_queue.unlock();
    */
  }

  static void popMessage(dyad_Event *e) {
    /*
    msg_queue.lock();
    msg_queue.pop();
    msg_queue.unlock();
    */
  }

  dyad_Stream* _socket;

  std::mutex opponent_lock;
  NetworkState opponent_state;
  bool new_opponent_state;

  std::mutex msg_lock;
  std::queue<NetworkState> msg_queue;

  std::atomic_bool _is_terminated = {false};
  std::thread _network_thread;
};

class Server : public Peer {
public:
  Server(int port) : Peer(port) {
    dyad_addListener(_socket, DYAD_EVENT_ACCEPT, onAccept, NULL);
    dyad_listen(_socket, port);
  }
private:
  static void onAccept(dyad_Event *e) {
    dyad_addListener(e->remote, DYAD_EVENT_DATA,  onData,      NULL);
    dyad_addListener(e->remote, DYAD_EVENT_ACK,   popMessage,  NULL);
    dyad_addListener(e->remote, DYAD_EVENT_NACK,  sendMessage, NULL);
    dyad_addListener(e->remote, DYAD_EVENT_READY, sendMessage, NULL);
  }
};

class Client : public Peer {
public:
  Client(const char* addr, int port) : Peer(port) {
    dyad_addListener(_socket, DYAD_EVENT_DATA,    onData,      NULL);
    dyad_addListener(_socket, DYAD_EVENT_ACK,     popMessage,  NULL);
    dyad_addListener(_socket, DYAD_EVENT_NACK,    sendMessage, NULL);
    dyad_addListener(_socket, DYAD_EVENT_READY,   sendMessage, NULL);
    dyad_connect(_socket, addr, port);
  }
};

#endif // NETWORKING
