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

#include <vector>
#include <unordered_map>

#include <hash_ring.hpp>

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
    // std::unordered_map<unsigned int, unsigned int> forward;


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
    hr.add(p);

    p.address = 1023;
    p.port = 4;
    hr.add(p);

    hr.print_ring();
    printf("\n");

    p.address = 2050;
    p.port = 7;
    hr.add(p);

    hr.print_ring();

    return 0;
}
