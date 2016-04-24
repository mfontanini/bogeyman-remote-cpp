#include <iostream>
#include <thread>
#include <boost/asio/io_service.hpp>
#include "reverse_server_connector.h"

using namespace std;

using boost::asio::io_service;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <server> <port>" << endl;
        return 1;
    }

    io_service service;
    io_service::work worker(service);
    thread th([&]() {
        service.run();
    });
    ReverseServerConnector connector(argv[1], stoi(argv[2]), service);

    connector.run();
    service.stop();
    th.join(); 
}