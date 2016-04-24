#include <boost/asio/write.hpp>
#include "tunnel.h"
#include "command_channel.h"

using std::bind;
using std::lock_guard;
using std::mutex;
using std::weak_ptr;

using boost::asio::io_service;
using boost::asio::ip::address;
using boost::asio::ip::tcp;

using boost::system::error_code;

Tunnel::Tunnel(weak_ptr<CommandChannel> channel, uint32_t id, io_service& service,
               const address& address, uint16_t port) 
: cmd_channel_(move(channel)), id_(id), socket_(service), target_endpoint_(address, port) {

}

void Tunnel::start() {
    using namespace std::placeholders;
    socket_.async_connect(target_endpoint_, bind(&Tunnel::handle_connect, shared_from_this(), _1));
}

void Tunnel::close() {
    error_code dummy;
    socket_.shutdown(tcp::socket::shutdown_both, dummy);
    socket_.close(dummy);
}

void Tunnel::send_data(DataType data) {
    lock_guard<mutex> _(pending_data_mutex_);
    pending_data_.push(move(data));
    if (!currently_sending_) {
        currently_sending_ = true;
        send_buffered_data();
    }
}

void Tunnel::send_buffered_data() {
    using namespace std::placeholders;
    DataType& data = pending_data_.front();
    async_write(socket_, boost::asio::buffer(data),
                bind(&Tunnel::handle_write_data, shared_from_this(), _1, _2));
}

void Tunnel::read_data() {
    using namespace std::placeholders;
    socket_.async_read_some(boost::asio::buffer(buffer_),
                            bind(&Tunnel::handle_read_data, shared_from_this(), _1, _2));
}

void Tunnel::handle_connect(const error_code& error) {
    if (auto channel = cmd_channel_.lock()) {
        if (error) {
            channel->notify_status(id_, STATUS_CONNECTION_REFUSED);
        }
        else {
            channel->notify_status(id_, STATUS_SUCCESS);
            read_data();
        }
    }
}

void Tunnel::handle_read_data(const error_code& error, size_t bytes_transferred) {
    if (auto channel = cmd_channel_.lock()) {
        if (error) {
            channel->remove_tunnel(id_);
        }
        else {
            DataType data(buffer_.begin(), buffer_.begin() + bytes_transferred);
            channel->forward_data(id_, move(data));
            read_data();
        }
    }
}

void Tunnel::handle_write_data(const error_code& error, size_t bytes_transferred) {
    if (auto channel = cmd_channel_.lock()) {
        if (error) {
            channel->remove_tunnel(id_);
        }
        else {
            lock_guard<mutex> _(pending_data_mutex_);
            pending_data_.pop();
            if (pending_data_.empty()) {
                currently_sending_ = false;
            }
            else {
                send_buffered_data();
            }
        }
    }
}
