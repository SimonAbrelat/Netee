#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <fpm/fixed.hpp>

#include "networking.hpp"
#include "collider.hpp"
#include "graphics.hpp"
#include "physics.hpp"

using f16 = fpm::fixed_16_16;

const char* HOST = "127.0.0.1";
const uint PORT = 8080;


std::shared_ptr<Graphics> graphics (new Graphics());
std::shared_ptr<Physics> physics (new Physics());
std::shared_ptr<Peer> network;

int main(int argc, char** argv) {
  if (argc < 2) {
    network = std::shared_ptr<Peer>(new Client(HOST, PORT));
  } else {
    network = std::shared_ptr<Peer>(new Server(PORT));
  }

  network->start();
  physics->run(network);
  graphics->update(physics);
  physics->abort();
}
