#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <errno.h>

#include <ipc.h>
#include <networking.h>

#include <string>
#include <vector>
#include <unordered_map>

#include <hash_ring.hpp>

#define PORT 8192
#define BUFFSIZE 1024
#define MAXEVENTS 32


int listening_unix_socket(std::string unix_address, int n)
{
    int fd = -1;
    struct sockaddr_un addr_info = {0};

    fd = socket(AF_UNIX, SOCK_STREAM, 0);

    addr_info.sun_family = AF_UNIX;
    addr_info.sun_path[0] = '\0';
    strcpy(addr_info.sun_path + 1, unix_address.c_str());
    
    bind(fd, (const struct sockaddr *)&addr_info, sizeof(addr_info.sun_family) + 1 + unix_address.length());

    listen(fd, n);

    return fd;
}

int listening_inet_socket(uint16_t port, int n)
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

void handle_ctl(int listen_unix_sock_fd, HashRing &hash_ring)
{

}

void handle_connect(int epoll_fd, int listen_inet_sock_fd, HashRing &hash_ring, std::unordered_map<unsigned int, unsigned int> &forward)
{
    int client_fd, server_fd;
    int flags;
    socklen_t addr_len;
    sockaddr_in addr_info;
    epoll_event event;
    Endpoint client_endpoint, server_endpoint;

    addr_len = sizeof(addr_info);

    client_fd = accept(listen_inet_sock_fd, (sockaddr *)&addr_info, &addr_len);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    client_endpoint.address = ntohl(addr_info.sin_addr.s_addr);
    client_endpoint.port = ntohs(addr_info.sin_port);
    printf("%x %d\n", client_endpoint.address, client_endpoint.port);

    server_endpoint = hash_ring.distribute(client_endpoint);
    printf("%x %d\n", server_endpoint.address, server_endpoint.port);

    addr_info.sin_addr.s_addr = htonl(server_endpoint.address);
    addr_info.sin_port = htons(server_endpoint.port);

    connect(server_fd, (const sockaddr *)&addr_info, sizeof(addr_info));

    forward[client_fd] = server_fd;
    forward[server_fd] = client_fd;

    flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    event.events = EPOLLIN;

    event.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
}

void handle_disconnect(int epoll_fd, int fd, std::unordered_map<unsigned int, unsigned int> &forward)
{
    int other_fd;

    other_fd = forward[fd];

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, other_fd, NULL);

    forward.erase(fd);
    forward.erase(other_fd);

    close(fd);
    close(other_fd);
}

void handle_traffic(int epoll_fd, int in_fd, std::unordered_map<unsigned int, unsigned int> &forward)
{
    int out_fd;
    ssize_t received_n;
    char buff[1024];

    received_n = recv(in_fd, buff, sizeof(buff), 0);

    if (received_n <= 0) {
        handle_disconnect(epoll_fd, in_fd, forward);

        return;
    }

    out_fd = forward[in_fd];

    while (received_n > 0) {
        send(out_fd, buff, received_n, 0);
        received_n = recv(in_fd, buff, sizeof(buff), 0);
    }
}

int main()
{
    bool healthy = true;
    int epoll_fd;
    int lis_unix_sock_fd, lis_inet_sock_fd;
    struct epoll_event event, events[MAXEVENTS];
    std::unordered_map<unsigned int, unsigned int> forward;
    HashRing hash_ring;


    epoll_fd = epoll_create(1);

    lis_unix_sock_fd = listening_unix_socket("loadbalancer", 4);
    lis_inet_sock_fd = listening_inet_socket(PORT, 4);

    if (epoll_fd == -1 || lis_unix_sock_fd == -1 || lis_inet_sock_fd == -1) {
        printf("error...\n");
        close(lis_inet_sock_fd);
        close(lis_unix_sock_fd);
        close(epoll_fd);
        return 0;
    }

    event.events = EPOLLIN;

    event.data.fd = STDIN_FILENO;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event);

    event.data.fd = lis_unix_sock_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, lis_unix_sock_fd, &event);

    event.data.fd = lis_inet_sock_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, lis_inet_sock_fd, &event);

    hash_ring.add(Endpoint(ntohl(inet_addr("127.0.0.1")), 8000));
    hash_ring.add(Endpoint(ntohl(inet_addr("127.0.0.1")), 9000));

    while (healthy) {
        int events_n = epoll_wait(epoll_fd, events, MAXEVENTS, -1);

        for (int i = 0; i < events_n; ++i) {
            int fd = events[i].data.fd;

            if (fd == STDIN_FILENO) {
                healthy = false;
            } else if (fd == lis_unix_sock_fd) {
                handle_ctl(lis_unix_sock_fd, hash_ring);
                printf("ctl...\n");
            } else if (fd == lis_inet_sock_fd) {
                handle_connect(epoll_fd, lis_inet_sock_fd, hash_ring, forward);
                printf("connect...\n");
            } else {
                handle_traffic(epoll_fd, fd, forward);
                printf("traffic...\n");
            }
        }
    }

    close(lis_inet_sock_fd);
    close(lis_unix_sock_fd);

    close(epoll_fd);

    return 0;
}
