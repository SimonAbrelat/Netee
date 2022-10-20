#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <fpm/fixed.hpp>

#include "server.hpp"
#include "graphics.hpp"
#include "collider.hpp"
#include "physics.hpp"

using f16 = fpm::fixed_16_16;

std::shared_ptr<Graphics> graphics (new Graphics());
std::shared_ptr<Physics> physics (new Physics());

using boost::asio::ip::tcp;
enum { max_length = 1024 };

const char* host = "127.0.0.1";
const char* port = "8080";

int make_client() {
//
// blocking_tcp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2021 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
    try {
      boost::asio::io_context io_context;

      tcp::socket s(io_context);
      tcp::resolver resolver(io_context);
      boost::asio::connect(s, resolver.resolve(host, port));

      for(;;) {
        std::cout << "Enter message: ";
        char request[max_length];
        std::cin.getline(request, max_length);
        size_t request_length = std::strlen(request);
        boost::asio::write(s, boost::asio::buffer(request, request_length));

        char reply[max_length];
        size_t reply_length = boost::asio::read(s,
            boost::asio::buffer(reply, request_length));
        std::cout << "Reply is: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";
      }
    }
    catch (std::exception& e) {
      std::cerr << "Exception: " << e.what() << "\n";
    }
  return 0;
}



int make_server() {
  try {
    boost::asio::io_context io_context;

    server s(io_context, std::atoi(port));

    io_context.run();
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

int main(int argc, char** argv) {
  /*
  Collider col1{f16(2),f16(2),f16(3),f16(3)};
  Collider col2{f16(1),f16(1),f16(2),f16(2)};

  std::cout << "Is colliding: " << col1.is_colliding(col2) << '\n';
  */

  /*
  if (argc < 2) {
      return make_client();
  }

  std::string input {argv[1]};
  boost::algorithm::to_lower(input);
  if (input.compare("server") == 0) {
      return make_server();
  } else {
      return make_client();
  }
  */
  physics->run();
  graphics->update(physics);
  physics->abort();
}
