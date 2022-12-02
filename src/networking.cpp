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
   if (enet_initialize () != 0) {
      std::cerr << "An error occurred while initializing ENet.\n";
   }
}

Peer::~Peer() {
   stop();
   enet_deinitialize();
}

#ifdef DEBUG
using std::chrono::operator""ms;

void Peer::debugloop(ENetHost* sock) {
   while (_is_debug) {
      const auto next_cycle = std::chrono::steady_clock::now() + 10ms;
      msg_lock.lock();
      for (auto msg = msg_queue.begin(); msg != msg_queue.end(); ++msg) {
         if (msg->time == 0) {
            std::cout << "SENDING: " << msg->msg.frame << '\n';
            ENetPacket* packet = enet_packet_create (NetworkState::serialize(msg->msg).data(), PACKET_SIZE, ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast (sock, 1, packet);
         }
         msg->time--;
      }
      msg_lock.unlock();
      std::this_thread::sleep_until(next_cycle);
   }
}
#endif

void Peer::networkloop(ENetHost* sock) {
   std::cout << "IN THREAD\n";
   ENetEvent event;
   NetworkState new_state;
   int status = 0;
   while (_is_terminated == false) {
      while ((status = enet_host_service (sock, &event, 10)) > 0) {
         switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
               std::clog << "Connected: " << event.peer->address.host << ", "  << event.peer->address.port << '\n';
               break;
            case ENET_EVENT_TYPE_RECEIVE:
               if (event.packet->dataLength == SYNC.size()) {
                  need_sync = true;
                  goto CLEANUP;
               }
               if (event.packet->dataLength != PACKET_SIZE) {
                  goto CLEANUP;
               }
               new_state = NetworkState::deserialize(reinterpret_cast<char*>(event.packet->data));
               // Check checksum
               if (!new_state.valid) {
                  std::clog << "BROKEN CHECKSUM\n";
                  goto CLEANUP;
               }
               for (auto it = opponent_states.cbegin(); it != opponent_states.cend(); ++it) {
                  if (it->frame == new_state.frame) {
                     goto CLEANUP;
                  }
               }
               opponent_states.push_front(new_state);
               new_states = true;
CLEANUP:
               enet_packet_destroy (event.packet);
               break;

            case ENET_EVENT_TYPE_DISCONNECT:
               printf ("%s disconnected.\n", ((char*)(event.peer -> data)));
               event.peer -> data = NULL;
               _is_terminated = true;
         }
         break;
      }
   }
}

void Peer::stop() {
   _is_terminated = true;
   _recv_thread.join();
#ifdef DEBUG
   _is_debug = false;
   _debug_thread.join();
#endif
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

bool Peer::needSync() {
   bool ret = need_sync;
   need_sync = false;
   return ret;
}

Server::~Server() {
   enet_host_destroy(server);
}

void Server::sendState(NetworkState state) {
#ifdef DEBUG
   msg_lock.lock();
   if (msg_queue.size() == MAX_MSG_BUFFER) {
      msg_queue.pop_back();
   }
   msg_queue.push_front(DebugMsg { state, rand()%10 });
   msg_lock.unlock();
#endif
#ifndef DEBUG
   ENetPacket * packet = enet_packet_create (NetworkState::serialize(state).data(), PACKET_SIZE, ENET_PACKET_FLAG_RELIABLE);
   enet_host_broadcast (server, 1, packet);
#endif
}

void Server::sendSync() {
   ENetPacket * packet = enet_packet_create (SYNC.c_str(), SYNC.size(), ENET_PACKET_FLAG_RELIABLE);
   enet_host_broadcast (server, 1, packet);
}

bool Server::start() {
   ENetAddress address;
   address.host = ENET_HOST_ANY;
   address.port = _port;

   server = enet_host_create (&address /* the address to bind the server host to */,
                              1        /* allow up to 32 clients and/or outgoing connections */,
                              2        /* allow up to 2 channels to be used, 0 and 1 */,
                              0        /* assume any amount of incoming bandwidth */,
                              0        /* assume any amount of outgoing bandwidth */);
   if (server == NULL) {
      std::cerr << "An error occurred while trying to create an ENet server host.\n";
      return false;
   }

   bool cont = false;
   ENetEvent event;
   while (!cont) {
      while (enet_host_service (server, &event, 10) > 0) {
         if (event.type == ENET_EVENT_TYPE_CONNECT) {
            std::clog << "Connected: " << event.peer->address.host << ", "  << event.peer->address.port << '\n';
            cont = true;
            break;
         }
      }
   }

   try {
      _recv_thread = std::thread(&Peer::networkloop, this, server);
#ifdef DEBUG
      _debug_thread = std::thread(&Peer::debugloop, this, server);
#endif
   } catch(std::exception e) {
      return false;
   }
   return true;
}

Client::~Client() {
   enet_host_destroy(client);
}

void Client::sendState(NetworkState state) {
#ifdef DEBUG
   msg_lock.lock();
   if (msg_queue.size() == MAX_MSG_BUFFER) {
      msg_queue.pop_back();
   }
   msg_queue.push_front(DebugMsg { state, rand()%10 });
   msg_lock.unlock();
#endif
#ifndef DEBUG
   ENetPacket * packet = enet_packet_create (NetworkState::serialize(state).data(), PACKET_SIZE, ENET_PACKET_FLAG_RELIABLE);
   enet_peer_send (server, 0, packet);
#endif
}

void Client::sendSync() {
   ENetPacket * packet = enet_packet_create (SYNC.c_str(), SYNC.size(), ENET_PACKET_FLAG_RELIABLE);
   enet_peer_send (server, 0, packet);
}

bool Client::start() {
   client = enet_host_create (NULL /* create a client host */,
                              1 /* only allow 1 outgoing connection */,
                              2 /* allow up 2 channels to be used, 0 and 1 */,
                              0 /* assume any amount of incoming bandwidth */,
                              0 /* assume any amount of outgoing bandwidth */);

   if (client == NULL) {
      std::cerr << "An error occurred while trying to create an ENet client host.\n";
      return false;
   }

   ENetAddress address;
   ENetEvent event;

   enet_address_set_host(&address, _host);
   address.port = _port;
   /* Initiate the connection, allocating the two channels 0 and 1. */
   server = enet_host_connect (client, & address, 2, 0);
   if (server == NULL) {
      std::cerr << "No available peers for initiating an ENet connection.\n";
      return false;
   }

   if (enet_host_service (client, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
      std::clog << "Connected: " << event.peer->address.host << ", "  << event.peer->address.port << '\n';
      server= event.peer;
   } else {
      std::cerr << "No connection\n";
      enet_peer_reset (server);
      return false;
   }

   try {
      _recv_thread = std::thread(&Peer::networkloop, this, client);
#ifdef DEBUG
      _debug_thread = std::thread(&Peer::debugloop, this, client);
#endif
   } catch(std::exception e) {
      return false;
   }
   return true;
}
