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
   UDT::close(sock);
   UDT::cleanup();
}


void Peer::recvloop() {
   std::cout << "IN THREAD\n";
   const int size = 100;
   char* data = new char[100];
   while (!_is_terminated) {
      // Blocking call
      std::cout << "WAITING FOR DATA\n";
      if (UDT::ERROR == UDT::recv(sock, data, size, 0)) {
         goto EXIT;
      }

      // TODO: PROCESS DATA
      std::cout << data;
      memset(data, 0, size);
   }

EXIT:
   delete [] data;
   UDT::close(sock);
}

void Peer::stop() {
   _is_terminated = true;
}

bool Peer::sendState(NetworkState& state) {
   msg_lock.lock();
   if (msg_queue.size() == MAX_MSG_BUFFER) {
   msg_queue.pop();
   }
   msg_queue.push(state);
   msg_lock.unlock();
   return UDT::ERROR == UDT::send(sock, NetworkState::serialize(state).data(), PACKET_SIZE, 0);
}

NetworkState Peer::readState() {
   opponent_lock.lock();
   NetworkState ret = opponent_state;
   new_opponent_state = false;
   opponent_lock.unlock();
   return ret;
}

bool Peer::newData() {
   opponent_lock.lock();
   bool ret = new_opponent_state;
   opponent_lock.unlock();
   return ret;
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
   sock = UDT::socket(res->ai_family, res->ai_socktype, res->ai_protocol);

   if (UDT::ERROR == UDT::bind(sock, res->ai_addr, res->ai_addrlen)) {
      std::cout << "bind: " << UDT::getlasterror().getErrorMessage() << '\n';
      return false;
   }

   freeaddrinfo(res);
   if (UDT::ERROR == UDT::listen(sock, 10)) {
      std::cout << "listen error\n";
      return false;
   }

   sockaddr_storage clientaddr;
   int clientaddr_len = sizeof(clientaddr);


   std::cout << "WAITING FOR CONNECT\n";
   UDTSOCKET recv;
   if (UDT::INVALID_SOCK == (recv = UDT::accept(sock, (sockaddr*)&clientaddr, &clientaddr_len))) {
      std::cout << "accept error\n";
      return false;
   }

   try {
      _recv_thread = std::thread(&Peer::recvloop, this); // Makes a copy
   } catch(std::exception e) {
      return false;
   }
   return true;
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

   UDTSOCKET sock = UDT::socket(local->ai_family, local->ai_socktype, local->ai_protocol);

   #ifdef WIN32
   UDT::setsockopt(sock, 0, UDT_MSS, new int(1052), sizeof(int));
   #endif

   freeaddrinfo(local);

      if (0 != getaddrinfo(_host, _port, &hints, &peer)) {
      std::cout << "incorrect server\n";
      return false;
   }

   if (UDT::ERROR == UDT::connect(sock, peer->ai_addr, peer->ai_addrlen)) {
      std::cout << "Connection failed\n";
      return false;
   }

   std::cout << "CONNECTION SUCCESS\n";

   freeaddrinfo(peer);

   try {
      _recv_thread = std::thread(&Peer::recvloop, this); // Makes a copy
   } catch(std::exception e) {
      return false;
   }
   return true;
}
