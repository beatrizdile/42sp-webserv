#include "webserv.h"

int main() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    char *buf = (char *)malloc(BUF_SIZE);
    struct sockaddr_in server_addr;
    struct epoll_event event;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    };

    if (listen(socket_fd, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(1);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(1);
    }

    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
        perror("epoll_ctl");
        exit(1);
    }

    while (true) {
        struct epoll_event events[MAX_CLIENTS];
        int event_count = epoll_wait(epoll_fd, events, MAX_CLIENTS, -1);
        if (event_count == -1) {
            perror("epoll_wait");
            exit(1);
        }

        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == socket_fd) {
                int client_socket = accept(socket_fd, NULL, NULL);
                if (client_socket == -1) {
                    perror("accept");
                    continue;
                }

                std::cout << "New connection with client " << client_socket << std::endl;
                int flags = fcntl(client_socket, F_GETFL, 0);
                fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

                event.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
                event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
                    perror("epoll_ctl");
                }
            } else if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR || events[i].events & EPOLLRDHUP) {
                std::cout << "Connection closed with client file descriptor " << events[i].data.fd << std::endl;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1) {
                    perror("epoll_ctl");
                }
                close(events[i].data.fd);
            } else if (events[i].events & EPOLLIN) {
                int bytes_read = recv(events[i].data.fd, buf, BUF_SIZE, 0);
                if (bytes_read == -1) {
                    perror("recv");
                    continue;
                } else if (bytes_read == 0) {
                    std::cout << "Connection closed with client file descriptor " << events[i].data.fd << std::endl;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1) {
                        perror("epoll_ctl");
                    }
                    close(events[i].data.fd);
                    continue;
                }
                buf[bytes_read] = '\0';
                printf("%s\n", buf);
                if (send(events[i].data.fd, "HTTP/1.1 200 OK\nContent-length: 46\nContent-Type: text/html\n\n<html><body><H1>Hello world</H1></body></html>", 107, 0) == -1) {
                    perror("send");
                }
            } else {
                std::cout << "Unexpected event " << events[i].events << std::endl;
            }
        }
    }

    free(buf);
    close(epoll_fd);
    close(socket_fd);
}
