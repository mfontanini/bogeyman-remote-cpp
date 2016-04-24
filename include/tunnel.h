#ifndef BOGEYMAN_TUNNEL_H
#define BOGEYMAN_TUNNEL_H

#include <memory>
#include <cstdint>
#include <array>
#include <vector>
#include <mutex>
#include <queue>
#include <boost/asio/ip/tcp.hpp>

namespace boost { namespace asio { 
    namespace ip { class address; }
    class io_service;
} }

class CommandChannel;

class Tunnel : public std::enable_shared_from_this<Tunnel> {
public:
    static constexpr int STATUS_CONNECTION_REFUSED = 5;
    static constexpr int STATUS_SUCCESS = 0;

    using DataType = std::vector<uint8_t>;

    Tunnel(std::weak_ptr<CommandChannel> channel, uint32_t id, boost::asio::io_service& service,
           const boost::asio::ip::address& address, uint16_t port);

    void start();
    void close();
    void send_data(DataType data);
private:
    using WeakSelf = std::weak_ptr<Tunnel>;
    using BufferType = std::array<uint8_t, 2048>;

    void send_buffered_data();
    void read_data();
    void handle_connect(const boost::system::error_code& error);
    void handle_read_data(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write_data(const boost::system::error_code& error, size_t bytes_transferred);

    std::weak_ptr<CommandChannel> cmd_channel_;
    uint32_t id_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::endpoint target_endpoint_;
    BufferType buffer_;
    std::queue<DataType> pending_data_;
    std::mutex pending_data_mutex_;
    bool currently_sending_ = false;
};

#endif // BOGEYMAN_TUNNEL_H
