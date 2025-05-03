#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include <sys/socket.h>
#include <sys/epoll.h>

#include <sys/un.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#include <map>
#include <set>

#include <vector>

#include <stdint.h>

#include <iostream>

#include <vector>
#include <unordered_map>

#define MAX_SERVERS_N 1024
#define REPLICA_N 2


struct HashNode {
    unsigned int id;
    unsigned long hash;
};

struct Endpoint {
    uint32_t address;
    uint16_t port;

    bool operator==(struct Endpoint &other)
    {
        return this->address == other.address && this->port == other.port;
    }
};

unsigned long id_hash(unsigned int id)
{
    unsigned long ulong_a = id;

	ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
	ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
	ulong_a = (ulong_a >> 16u) ^ ulong_a;

	return ulong_a;
}

unsigned long endpoint_hash(Endpoint &endpoint)
{
    unsigned long ulong_a = endpoint.address;

	ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
    ulong_a = (ulong_a >> 16u) ^ endpoint.port;
	ulong_a = ((ulong_a >> 16u) ^ ulong_a) * 0x45d9f3b;
	ulong_a = (ulong_a >> 16u) ^ ulong_a;

	return ulong_a;
}

class HashRing {
private:
    unsigned int n;
    
    std::unordered_map<unsigned int, Endpoint> endpoints;
    std::vector<HashNode> nodes;

    // std::unordered_map<Endpoint, unsigned int> ids;
    // std::unordered_set<unsigned int> used_ids;
    // std::vector<HashNode> nodes;

public:
    HashRing(unsigned int servers_n)
    {
        if (servers_n > MAX_SERVERS_N)
            servers_n = MAX_SERVERS_N;

        this->n = servers_n;
    }

private:
    unsigned int _find_smallest_bigger(unsigned long hash)
    {
        int l, m, r;
        std::vector<HashNode> &nodes = this->nodes;

        l = 0;
        r = this->nodes.size() - 1;
        m = l + ((r - l) / 2);

        while (l < r && nodes[m].hash != hash) {
            if (nodes[m].hash < hash)
                l = m + 1;
            else
                r = m;

            m = l + ((r - l) / 2);
        }

        return m;
    }

public:
    void add_server(Endpoint &server_endpoint)
    {
        unsigned int id;
        std::unordered_map<unsigned int, Endpoint> &endpoints = this->endpoints;
        std::vector<HashNode> &nodes = this->nodes;

        if (endpoints.size() >= this->n)
            return;

        for (auto iter = endpoints.begin(); iter != endpoints.end(); ++iter)
            if ((*iter).second == server_endpoint)
                return;

        for (unsigned int i = 0; i < this->n; ++i) {
            if (endpoints.find(i) == endpoints.end()) {
                endpoints.insert({i, server_endpoint});
                id = i;

                break;
            }
        }

        for (int i = 0; i < REPLICA_N; ++i) {
            unsigned long hash = id_hash(id + i * this->n);

            if (nodes.empty() || (hash >= (*(--nodes.end())).hash))
                nodes.push_back((HashNode){id, hash});
            else
                nodes.insert(nodes.begin() + this->_find_smallest_bigger(hash), (HashNode){id, hash});
        }
    }

    void remove_server(Endpoint &server_endpoint)
    {
        bool found = false;
        unsigned int id;

        for (auto iter = this->endpoints.begin(); iter != this->endpoints.end(); ++iter)
            if ((*iter).second == server_endpoint) {
                id = (*iter).first;
                this->endpoints.erase(iter);

                found = true;

                break;
            }

        if (!found)
            return;

        for (auto iter = this->nodes.begin(); iter != this->nodes.end(); ++iter)
            if ((*iter).id == id) {
                this->nodes.erase(iter);
                --iter;
            }
    }

    Endpoint get_server(Endpoint &client_endpoint)
    {

    }

    void print_ring()
    {
        for (auto node : nodes)
            std::cout << node.id << ' ' << node.hash << '\n';
    }
};


#define PORT 8192
#define BUFFSIZE 1024
#define MAXEVENTS 32


int listening_inet_socket(int n, uint16_t port)
{
    int fd = -1;
    struct sockaddr_in addr_info = {0};

    fd = socket(AF_INET, SOCK_STREAM, 0);

    addr_info.sin_family = AF_INET;
    addr_info.sin_addr.s_addr = INADDR_ANY;
    addr_info.sin_port = htons(port);

    bind(fd, (const struct sockaddr *)&addr_info, sizeof(addr_info));

    listen(fd, n);

    return fd;
}

int main(int argc, char* argv[])
{
    // int i;
    // int listen_sock_fd, new_sock_fd, fd;
    // int epoll_fd;
    // int events_n;
    // char buff[BUFFSIZE];
    // struct epoll_event event, events[MAXEVENTS];
    // std::map<unsigned int, unsigned int> forward;


    // listen_sock_fd = listening_inet_socket(4, PORT);

    // epoll_fd = epoll_create(1);

    // event.data.fd = listen_sock_fd;
    // event.events = EPOLLIN;
    // epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock_fd, &event);

    // while (1) {
    //     events_n = epoll_wait(epoll_fd, events, MAXEVENTS, -1);

    //     for (i = 0; i < events_n; ++i) {
    //         if (events[i].data.fd == listen_sock_fd) {
    //             // create new socket and get source address
    //             // compute server address
    //             // create new socket for that server


    //             new_sock_fd = accept(listen_sock_fd, NULL, NULL);

    //             event.data.fd = new_sock_fd;
    //             epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_sock_fd, &event);
    //         } else {
    //             // recv data from socket
    //             // forward traffic to its corresponding socket

    //             fd = events[i].data.fd;

    //             if (recv(fd, buff, BUFFSIZE, 0) == 0) {
    //                 epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    //                 close(fd);
    //             } else {
    //                 printf("%s\n", buff);
    //             }

    //         }
    //     }
    // }

    Endpoint p;
    HashRing hr(4);

    p.address = 1024;
    p.port = 2;
    hr.add_server(p);

    p.address = 1023;
    p.port = 4;
    hr.add_server(p);

    hr.print_ring();
    std::cout << '\n';

    p.address = 2050;
    p.port = 7;
    hr.add_server(p);

    hr.print_ring();

    return 0;
}
