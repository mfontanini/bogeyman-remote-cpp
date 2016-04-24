#ifndef BOGEYMAN_REVERSE_SERVER_CONNECTOR_H
#define BOGEYMAN_REVERSE_SERVER_CONNECTOR_H

#include <string>
#include <cstdint>

namespace boost { namespace asio { class io_service; } }

class ReverseServerConnector {
public:
    ReverseServerConnector(const std::string& hostname, uint16_t port,
                           boost::asio::io_service& service);

    void run();
private:


    std::string hostname_;
    uint16_t port_;
    boost::asio::io_service& io_service_;
};

#endif // BOGEYMAN_REVERSE_SERVER_CONNECTOR_H
