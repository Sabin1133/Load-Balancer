#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "utils/ipc_utils.hpp"
#include "utils/inetw_utils.hpp"

#include "conn_engine.hpp"

#define EVENTS_MAX_NR 32
#define INPUT_BUFF_MAX_SIZE 32


enum class InputToken
{
    NONE,
    START,
    STOP,
    ADD,
    RMV,
    LIST
};

enum InputToken parse_input_buffer(char *buffer)
{
    if (strcmp(buffer, "start") == 0)
        return InputToken::ADD;

    if (strcmp(buffer, "stop") == 0)
        return InputToken::ADD;

    if (strcmp(buffer, "add") == 0)
        return InputToken::ADD;

    if (strcmp(buffer, "rmv") == 0)
        return InputToken::RMV;

    if (strcmp(buffer, "list") == 0)
        return InputToken::ADD;

    return InputToken::NONE;
}

int check_address_buffer(char *buffer)
{
    int bytes;
    uint32_t address;

    bytes = sscanf(buffer, "%hhu.%hhu.%hhu.%hhu", (char *)&address + 3, (char *)&address + 2, (char *)&address + 1, (char *)&address);

    if (bytes != 4)
        return -1;

    return 0;
}

ConnectionEngine::ConnectionEngine() : epoll_fd(-1), lis_unix_sock(-1), lis_inet_sock(-1) {}

int ConnectionEngine::setup(std::string unix_address, uint32_t inet_address, uint16_t inet_port)
{
    int epoll_fd;
    int lis_unix_sock, lis_inet_sock;
    struct epoll_event event;

    epoll_fd = epoll_create(1);

    if (epoll_fd == -1)
        return -1;

    lis_unix_sock = listening_unix_socket(unix_address, 4);
    lis_inet_sock = listening_inet_socket(inet_address, inet_port, 4);

    if (lis_unix_sock == -1 || lis_inet_sock == -1) {
        close(lis_inet_sock);
        close(lis_unix_sock);
        close(epoll_fd);

        return -1;
    }

    event.events = EPOLLIN;

    event.data.fd = STDIN_FILENO;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &event);

    event.data.fd = lis_unix_sock;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, lis_unix_sock, &event);

    event.data.fd = lis_inet_sock;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, lis_inet_sock, &event);

    this->epoll_fd = epoll_fd;
    this->lis_unix_sock = lis_unix_sock;
    this->lis_inet_sock = lis_inet_sock;

    // remove later
    this->hash_ring.add(Endpoint(ntohl(inet_addr("127.0.0.1")), 8000));
    this->hash_ring.add(Endpoint(ntohl(inet_addr("127.0.0.1")), 9000));

    return 0;
}

void ConnectionEngine::handle_ctl()
{
    int rc;
    int lis_unix_sock = this->lis_unix_sock;
    int client_sock = -1;
    enum InputToken token;
    char buffer[INPUT_BUFF_MAX_SIZE];
    FILE *client_in_fh = NULL, *client_out_fh = NULL;

    client_sock = accept(lis_unix_sock, NULL, NULL);

    if (client_sock == -1)
        return;

    client_in_fh = fdopen(client_sock, "r");

    if (!client_in_fh) {
        close(client_sock);
        return;
    }

    client_out_fh = fdopen(client_sock, "w");

    if (!client_out_fh) {
        fclose(client_in_fh);
        close(client_sock);
        return;
    }

    rc = fscanf(client_in_fh, "%s", buffer);

    token = parse_input_buffer(buffer);

    uint32_t address;
    uint16_t port;

    switch (token) {
    case InputToken::START:
        break;
    case InputToken::STOP:
        break;
    case InputToken::ADD:
        printf("adding\n");

        rc = fscanf(client_in_fh, "%s", buffer);

        if (rc < 1)
            return;

        rc = inet_pton(AF_INET, buffer, (in_addr *)&address);

        if (rc == -1)
            return;

        rc = fscanf(client_in_fh, "%hu", &port);

        if (rc < 1)
            return;

        this->hash_ring.add(Endpoint(ntohl(address), port));

        inet_ntop(AF_INET, &address, buffer, sizeof(buffer));

        printf("adding %s, %hu\n", buffer, port);

        break;
    case InputToken::RMV:
        break;
    case InputToken::LIST:
        break;
    case InputToken::NONE:
        break;
    }

    while (rc > 0) {
        printf("%d\n", rc);
        fprintf(client_out_fh, "%s\n", buffer);

        rc = fscanf(client_in_fh, "%s", buffer);
    }

    fclose(client_out_fh);
    fclose(client_in_fh);
    close(client_sock);
}

void ConnectionEngine::handle_connect()
{
    int rc;
    int client_fd, server_fd;
    int flags;
    socklen_t addr_len;
    sockaddr_in addr_info;
    epoll_event event;
    Endpoint client_endpoint, server_endpoint;

    addr_len = sizeof(addr_info);

    client_fd = accept(this->lis_inet_sock, (sockaddr *)&addr_info, &addr_len);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    client_endpoint.address = ntohl(addr_info.sin_addr.s_addr);
    client_endpoint.port = ntohs(addr_info.sin_port);

    server_endpoint = hash_ring.distribute(client_endpoint);

    addr_info.sin_addr.s_addr = htonl(server_endpoint.address);
    addr_info.sin_port = htons(server_endpoint.port);

    rc = connect(server_fd, (const sockaddr *)&addr_info, sizeof(addr_info));

    if (rc == -1) {
        close(server_fd);
        close(client_fd);

        return;
    }

    flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    this->open_sockets.insert(client_fd);
    this->open_sockets.insert(server_fd);

    forward[client_fd] = server_fd;
    forward[server_fd] = client_fd;

    event.events = EPOLLIN;

    event.data.fd = client_fd;
    epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

    event.data.fd = server_fd;
    epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, server_fd, &event);
}

void ConnectionEngine::handle_disconnect(int fd)
{
    int other_fd;

    other_fd = this->forward[fd];

    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, other_fd, NULL);

    this->forward.erase(fd);
    this->forward.erase(other_fd);

    this->open_sockets.erase(fd);
    this->open_sockets.erase(other_fd);

    close(fd);
    close(other_fd);
}

void ConnectionEngine::handle_traffic(int in_fd)
{
    int out_fd;
    ssize_t received_n;
    char buff[1024];

    received_n = recv(in_fd, buff, sizeof(buff), 0);

    if (received_n <= 0) {
        this->handle_disconnect(in_fd);

        return;
    }

    out_fd = forward[in_fd];

    while (received_n > 0) {
        send(out_fd, buff, received_n, 0);
        received_n = recv(in_fd, buff, sizeof(buff), 0);
    }
}

void ConnectionEngine::run()
{
    bool alive = true;
    struct epoll_event events[EVENTS_MAX_NR];

    while (alive) {
        int events_n = epoll_wait(this->epoll_fd, events, EVENTS_MAX_NR, -1);

        for (int i = 0; i < events_n; ++i) {
            int fd = events[i].data.fd;

            if (fd == STDIN_FILENO) {
                alive = false;
            } else if (fd == this->lis_unix_sock) {
                printf("ctl...\n");
                this->handle_ctl();
            } else if (fd == this->lis_inet_sock) {
                printf("connect...\n");
                this->handle_connect();
            } else {
                printf("traffic...\n");
                this->handle_traffic(fd);
            }
        }
    }
}

void ConnectionEngine::cleanup()
{
    for (auto fd : this->open_sockets) {
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
    }

    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, this->lis_inet_sock, NULL);
    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, this->lis_unix_sock, NULL);

    close(this->lis_inet_sock);
    close(this->lis_unix_sock);
    close(this->epoll_fd);
}
