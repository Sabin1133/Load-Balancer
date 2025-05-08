#ifndef CONN_ENGINE_HPP
#define CONN_ENGINE_HPP

#include <stdint.h>

#include <string>
#include <unordered_set>
#include <unordered_map>

#include <hash_ring.hpp>


class ConnectionEngine {
private:
    int epoll_fd;
    int lis_unix_sock;
    int lis_inet_sock;

    std::unordered_set<int> open_sockets;
    std::unordered_map<int, int> forward;

    HashRing hash_ring;

public:
    ConnectionEngine();

private:
    void handle_ctl();
    void handle_connect();
    void handle_disconnect(int fd);
    void handle_traffic(int in_fd);

public:
    int setup(std::string unix_address, uint32_t inet_address, uint16_t inet_port);
    void run();
    void cleanup();
};

#endif /* conn_engine.hpp */
