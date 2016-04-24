#ifndef BOGEYMAN_COMMAND_CHANNEL_H
#define BOGEYMAN_COMMAND_CHANNEL_H

#include <array>
#include <limits>
#include <cstdint>
#include <string>
#include <map>
#include <stdexcept>
#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <boost/asio/ip/tcp.hpp>
#include "tunnel.h"

namespace Json { class Value; }

class CommandChannelException : public std::runtime_error {
public:
    CommandChannelException(const std::string& message)
    : std::runtime_error(message) {

    }
};

class CommandChannel : public std::enable_shared_from_this<CommandChannel> {
public:
    CommandChannel(boost::asio::ip::tcp::socket server_socket);

    void run();
    void stop();
    void notify_status(uint32_t tunnel_id, int status);
    void remove_tunnel(uint32_t tunnel_id);
    void forward_data(uint32_t tunnel_id, Tunnel::DataType data);
private:
    static constexpr size_t COMMAND_SIZE_FIELD = 2;
    static constexpr size_t MAX_PAYLOAD_SIZE = std::numeric_limits<uint16_t>::max();
    using CommandMap = std::map<std::string, void(CommandChannel::*)(const Json::Value&)>;
    static const CommandMap COMMANDS;
    using BufferType = std::array<char, MAX_PAYLOAD_SIZE>;
    using TunnelMap = std::map<uint32_t, std::shared_ptr<Tunnel>>;

    void enqueue_command(const Json::Value& command);
    void read_command_size();
    void read_command_payload(size_t command_size);
    void handle_command_size(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_command_payload(const boost::system::error_code& error, size_t bytes_transferred);
    // Command handlers
    void handle_connect(const Json::Value& command);
    void handle_stop(const Json::Value& command);
    void handle_sync(const Json::Value& command);

    boost::asio::ip::tcp::socket server_socket_;
    BufferType buffer_;
    TunnelMap tunnels_;
    std::mutex tunnels_mutex_;
    std::queue<std::string> outbound_command_queue_;
    std::mutex outbound_commands_mutex_;
    std::condition_variable outbound_commands_cond_;
    std::atomic<bool> running_;
};

#endif // BOGEYMAN_COMMAND_CHANNEL_H
