#include <utility>
#include <functional>
#include <boost/asio/buffer.hpp>
#include <boost/asio/read.hpp>
#include <json/value.h>
#include <json/reader.h>
#include <json/writer.h>
#ifdef _WIN32
    #include <Winsock2.h>
#else 
    #include <netinet/in.h>
#endif // _WIN32
#include "command_channel.h"
#include "base64.h"

using std::move;
using std::bind;
using std::weak_ptr;
using std::make_shared;
using std::mutex;
using std::lock_guard;
using std::string;

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::async_read;

using boost::system::error_code;

const CommandChannel::CommandMap CommandChannel::COMMANDS {
    { "connect", &CommandChannel::handle_connect },
    { "sync", &CommandChannel::handle_sync },
    { "stop", &CommandChannel::handle_stop }
};

CommandChannel::CommandChannel(tcp::socket server_socket) 
: server_socket_(move(server_socket)) {

}

void CommandChannel::run() {
    read_command_size();
}

void CommandChannel::notify_status(uint32_t tunnel_id, int status) {

}

void CommandChannel::remove_tunnel(uint32_t tunnel_id) {

}

void CommandChannel::forward_data(uint32_t tunnel_id, Tunnel::DataType data) {

}

void CommandChannel::read_command_size() {
    using namespace std::placeholders;
    async_read(server_socket_, boost::asio::buffer(buffer_, COMMAND_SIZE_FIELD),
               bind(&CommandChannel::handle_command_size, this, _1, _2));
}

void CommandChannel::read_command_payload(size_t command_size) {
    using namespace std::placeholders;
    async_read(server_socket_, boost::asio::buffer(buffer_, command_size),
               bind(&CommandChannel::handle_command_payload, this, _1, _2));
}

void CommandChannel::handle_command_size(const error_code& error, size_t) {
    if (error) {

    }
    else {
        uint16_t command_size;
        memcpy(&command_size, buffer_.data(), sizeof(command_size));
        command_size = ntohs(command_size);
        read_command_payload(command_size);
    }
}

void CommandChannel::handle_command_payload(const error_code& error, size_t bytes_transferred) {
    if (error) {

    }
    else {
        string command(buffer_.begin(), buffer_.begin() + bytes_transferred);
        Json::Value json_command;
        Json::Reader reader;
        if (!reader.parse(command, json_command)) {
            throw CommandChannelException("Malformed JSON string received");
        }
        if (!json_command.isMember("cmd")) {
            throw CommandChannelException("Invalid command received: missing 'cmd'");
        }
        string command_type = json_command["cmd"].asString();
        auto iter = COMMANDS.find(command_type);
        if (iter == COMMANDS.end()) {
            throw CommandChannelException("Invalid command received: invalid command type");
        }
        (this->*iter->second)(command);
    }
}

void CommandChannel::handle_connect(const Json::Value& command) {
    if (!command.isMember("id") || !command.isMember("addr") || !command.isMember("port")) {
        throw CommandChannelException("Invalid connect received: missing argument(s)");
    }
    uint32_t id = command["id"].asUInt();
    address target_address = address::from_string(command["addr"].asString());
    uint16_t target_port = command["port"].asUInt();

    weak_ptr<CommandChannel> weak_self(shared_from_this());
    // Construct tunnel and store it
    auto tunnel = make_shared<Tunnel>(weak_self, id, server_socket_.get_io_service(),
                                      target_address, target_port);
    lock_guard<mutex> _(tunnels_mutex_);
    tunnels_.emplace(id, move(tunnel));
}

void CommandChannel::handle_stop(const Json::Value& command) {

}

void CommandChannel::handle_sync(const Json::Value& command) {
    if (!command.isMember("id") || !command.isMember("data")) {
        throw CommandChannelException("Invalid sync received: missing argument(s)");
    }
    uint32_t id = command["id"].asUInt();
    string string_data = base64_decode(command["data"].asString());
    Tunnel::DataType data(string_data.begin(), string_data.end());

    lock_guard<mutex> _(tunnels_mutex_);
    auto iter = tunnels_.find(id);
    if (iter != tunnels_.end()) {
        iter->second->send_data(move(data));
    }
}
