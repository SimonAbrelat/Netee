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
const char* PORT = "8080";

std::shared_ptr<Graphics> graphics (new Graphics());
std::shared_ptr<Physics> physics (new Physics());
std::shared_ptr<Peer> network;

int main(int argc, char** argv) {
  /*
  Collider col1{f16(2),f16(2),f16(3),f16(3)};
  Collider col2{f16(1),f16(1),f16(2),f16(2)};

  std::cout << "Is colliding: " << col1.is_colliding(col2) << '\n';
  */

  if (argc < 2) {
    network = std::shared_ptr<Peer>(new Client(HOST, PORT));
  } else {
    network = std::shared_ptr<Peer>(new Server(PORT));
  }

  /*
  std::string input {argv[1]};
  boost::algorithm::to_lower(input);
  if (input.compare("server") == 0) {
      return make_server();
  } else {
      return make_client();
  }
  */
  network->start();
  physics->run(network);
  graphics->update(physics);
  physics->abort();
}
