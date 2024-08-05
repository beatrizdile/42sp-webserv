#include "webserv.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        exit(1);
    }

    Logger logger("Webserv");
    Config config = Config();
    try {
        config.loadConfig(argv[1]);
    } catch (std::exception &e) {
        logger.error() << "Error: " << e.what() << std::endl;
        exit(1);
    }

#ifndef __APPLE__
    logger.info() << "Starting server" << std::endl;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    std::vector<char> buffer = std::vector<char>(BUFFER_SIZE);
    struct sockaddr_in server_addr;
    struct epoll_event event;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        logger.perror("bind");
        exit(1);
    };

    if (listen(socket_fd, MAX_CLIENTS) == -1) {
        logger.perror("listen");
        exit(1);
    }

    logger.info() << "Server started on port " << PORT << std::endl;

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        logger.perror("epoll_create1");
        exit(1);
    }

    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1) {
        logger.perror("epoll_ctl");
        exit(1);
    }

    while (true) {
        struct epoll_event events[MAX_CLIENTS];
        int event_count = epoll_wait(epoll_fd, events, MAX_CLIENTS, -1);
        if (event_count == -1) {
            logger.perror("epoll_wait");
            exit(1);
        }

        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == socket_fd) {
                int client_socket = accept(socket_fd, NULL, NULL);
                if (client_socket == -1) {
                    logger.perror("accept");
                    continue;
                }

                logger.info() << "New connection with client " << client_socket << std::endl;
                int flags = fcntl(client_socket, F_GETFL, 0);
                fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

                event.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
                event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
                    logger.perror("epoll_ctl");
                }
            } else if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR || events[i].events & EPOLLRDHUP) {
                logger.info() << "Connection closed with client " << events[i].data.fd << std::endl;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1) {
                    logger.perror("epoll_ctl");
                }
                close(events[i].data.fd);
            } else if (events[i].events & EPOLLIN) {
                int bytes_read = recv(events[i].data.fd, buffer.data(), BUFFER_SIZE, 0);
                if (bytes_read == -1) {
                    logger.perror("recv");
                    continue;
                } else if (bytes_read == 0) {
                    logger.info() << "Connection closed with client " << events[i].data.fd << std::endl;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL) == -1) {
                        logger.perror("epoll_ctl");
                    }
                    close(events[i].data.fd);
                    continue;
                }
                buffer[bytes_read] = '\0';
                logger.info() << "Received " << bytes_read << " bytes from client " << events[i].data.fd << ": " << buffer.data() << std::endl;
                if (send(events[i].data.fd, "HTTP/1.1 200 OK\nContent-length: 46\nContent-Type: text/html\n\n<html><body><H1>Hello world</H1></body></html>", 107, 0) == -1) {
                    logger.perror("send");
                }
            } else {
                logger.info() << "Unexpected event " << events[i].events << " for client " << events[i].data.fd << std::endl;
            }
        }
    }

    close(epoll_fd);
    close(socket_fd);
#endif
}

