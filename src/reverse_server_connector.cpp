#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "reverse_server_connector.h"

using std::string;
using std::to_string;

using boost::asio::io_service;
using boost::asio::ip::tcp;

ReverseServerConnector::ReverseServerConnector(const string& hostname, uint16_t port,
                                               io_service& service)
: hostname_(hostname), port_(port), io_service_(service) {

}

void ReverseServerConnector::run() {
    tcp::resolver name_resolver(io_service_);
    tcp::resolver::query query(hostname_, to_string(port_));
    tcp::endpoint ep = *name_resolver.resolve(query);
    while (true) {
        tcp::socket server_socket(io_service_);
        server_socket.connect(ep);

    }
}
