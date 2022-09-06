#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/algorithm/string.hpp>

#include "server.hpp"

using namespace boost::asio;
using ip::tcp;
using std::cout;
using std::endl;

int make_client() {
    boost::asio::io_service io_service;
//socket creation
    tcp::socket socket(io_service);
//connection
    socket.connect( tcp::endpoint( boost::asio::ip::address::from_string("127.0.0.1"), 1234 ));
// request/message from client
    const string msg = "Hello from Client!\n";
    boost::system::error_code error;
    boost::asio::write( socket, boost::asio::buffer(msg), error );
    if( !error ) {
       cout << "Client sent hello message!" << endl;
    }
    else {
       cout << "send failed: " << error.message() << endl;
    }
 // getting response from server
    boost::asio::streambuf receive_buffer;
    boost::asio::read(socket, receive_buffer, boost::asio::transfer_all(), error);
    if( error && error != boost::asio::error::eof ) {
        cout << "receive failed: " << error.message() << endl;
    }
    else {
        const char* data = boost::asio::buffer_cast<const char*>(receive_buffer.data());
        cout << data << endl;
    }
    return 0;
}

int make_server() {
    try {
        boost::asio::io_service io_service;  
        Server server(io_service);
        io_service.run();
    }
    catch(std::exception& e) {
        std::cerr << e.what() << endl;
    }
    return 0;
}

int main(int argc, char** argv) {
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
}