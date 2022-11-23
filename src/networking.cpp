#ifndef WIN32
   #include <unistd.h>
   #include <cstdlib>
   #include <cstring>
   #include <netdb.h>
#else
   #include <winsock2.h>
   #include <ws2tcpip.h>
   #include <wspiapi.h>
#endif

#include <iostream>
#include <exception>
#include <chrono>

#include "networking.hpp"

Peer::Peer() {
   UDT::startup();
}

Peer::~Peer() {
   stop();
   UDT::cleanup();
}

void Peer::recvloop(UDTSOCKET recver) {
   std::cout << "IN THREAD\n";
   const int size = 100;
   char* data = new char[100];
   while (!_is_terminated) {
      // Blocking call
      int block_size = 0;
      if (UDT::ERROR == (block_size = UDT::recv(recver, data, size, 0))) {
         goto EXIT;
      }

      if (block_size < PACKET_SIZE) {
         continue;
      }
      opponent_lock.lock();
      NetworkState new_state = NetworkState::deserialize(data);
      if (!new_state.valid) {
         goto END_LOOP;
      }
      // TODO: Make sure this condition is correct
      // Cond 1: What about old duplicates? Probably handled in physics
      // Cond 2: Two new inputs on the same physics frame? I need to make a new input queue
      for (auto it = opponent_states.cbegin(); it != opponent_states.cend(); ++it) {
         if (it->frame == new_state.frame || new_state.frame == 0) {
            goto END_LOOP;
         }
      }
      opponent_states.push_front(new_state);
      new_states = true;
END_LOOP:
      opponent_lock.unlock();
      memset(data, 0, size);
   }

EXIT:
   delete [] data;
   UDT::close(recver);
   std::terminate();
}

void Peer::stop() {
   _is_terminated = true;
}

std::deque<NetworkState> Peer::readStates() {
   opponent_lock.lock();
   std::deque<NetworkState> ret = opponent_states;
   new_states = false;
   opponent_states.clear();
   opponent_lock.unlock();
   return ret;
}

bool Peer::newData() {
   return new_states;
}

Server::~Server() {
   UDT::close(serv);
   UDT::close(recv);
   UDT::close(send);
}

bool Server::sendState(NetworkState state) {
   msg_lock.lock();
   if (msg_queue.size() == MAX_MSG_BUFFER) {
      msg_queue.pop();
   }
   msg_queue.push(state);
   msg_lock.unlock();
   return UDT::ERROR != UDT::send(send, NetworkState::serialize(state).data(), PACKET_SIZE, 0);
}

bool Server::start() {
   addrinfo hints {};
   addrinfo* res;

   hints.ai_flags = AI_PASSIVE;
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;

   if (0 != getaddrinfo(NULL, _port, &hints, &res)) {
      std::cout << "illegal port number or port is busy.\n";
      return false;
   }

   // Set socket
   serv = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

   if (UDT::ERROR == UDT::bind(serv, res->ai_addr, res->ai_addrlen)) {
      std::cout << "bind: " << UDT::getlasterror().getErrorMessage() << '\n';
      return false;
   }

   freeaddrinfo(res);
   if (UDT::ERROR == UDT::listen(serv, 10)) {
      std::cout << "listen error\n";
      return false;
   }

   sockaddr_storage clientaddr;
   int clientaddr_len = sizeof(clientaddr);


   std::cout << "WAITING FOR CONNECT\n";
   if (UDT::INVALID_SOCK == (send = UDT::accept(serv, (sockaddr*)&clientaddr, &clientaddr_len))) {
      std::cout << "RECV socket error\n";
      return false;
   }

   if (UDT::INVALID_SOCK == (recv = UDT::accept(serv, (sockaddr*)&clientaddr, &clientaddr_len))) {
      std::cout << "SEND accept error\n";
      return false;
   }

   try {
      _recv_thread = std::thread(&Peer::recvloop, this, UDTSOCKET(recv));
   } catch(std::exception e) {
      return false;
   }
   return true;
}

Client::~Client() {
   UDT::close(send);
   UDT::close(recv);
}

bool Client::sendState(NetworkState state) {
   msg_lock.lock();
   if (msg_queue.size() == MAX_MSG_BUFFER) {
      msg_queue.pop();
   }
   msg_queue.push(state);
   msg_lock.unlock();
   return UDT::ERROR != UDT::send(send, NetworkState::serialize(state).data(), PACKET_SIZE, 0);
}


bool Client::start() {
   struct addrinfo hints, *local, *peer;

   memset(&hints, 0, sizeof(struct addrinfo));

   hints.ai_flags = AI_PASSIVE;
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;

   if (0 != getaddrinfo(NULL, _port, &hints, &local)) {
      std::cout << "incorrect port: " << _port << '\n';
      return false;
   }

   recv = UDT::socket(local->ai_family, local->ai_socktype, local->ai_protocol);
   send = UDT::socket(local->ai_family, local->ai_socktype, local->ai_protocol);

   #ifdef WIN32
      UDT::setsockopt(recv, 0, UDT_MSS, new int(1052), sizeof(int));
      UDT::setsockopt(send, 0, UDT_MSS, new int(1052), sizeof(int));
   #endif

   freeaddrinfo(local);

   if (0 != getaddrinfo(_host, _port, &hints, &peer)) {
      std::cout << "incorrect server\n";
      return false;
   }

   if (UDT::ERROR == UDT::connect(recv, peer->ai_addr, peer->ai_addrlen)) {
      std::cout << "RECV socket failed\n";
      return false;
   }
   if (UDT::ERROR == UDT::connect(send, peer->ai_addr, peer->ai_addrlen)) {
      std::cout << "SEND socket failed\n";
      return false;
   }

   std::cout << "CONNECTION SUCCESS\n";

   freeaddrinfo(peer);

   try {
      _recv_thread = std::thread(&Peer::recvloop, this, UDTSOCKET(recv));
   } catch(std::exception e) {
      return false;
   }
   return true;
}
