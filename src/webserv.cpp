
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>

#define BUF_SIZE 500
#define EPOLL_MAX_EVENTS 5

int main()
{
    int socket_fd;
    char *buf = (char *)malloc(BUF_SIZE);
    struct sockaddr_in server_addr;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(1);
    };
    if (listen(socket_fd, EPOLL_MAX_EVENTS) == -1) {
        perror("listen");
        exit(1);
    }

    int epoll_fd = epoll_create1(0);
    struct epoll_event event;

    event.events = EPOLLIN;
    event.data.fd = socket_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    

    while (true)
    {
        struct epoll_event events[EPOLL_MAX_EVENTS];
        int event_count = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, -1);

        for (int i = 0; i < event_count; i++) {
            if (events[i].data.fd == socket_fd) {
                int client_socket = accept(socket_fd, NULL, NULL);
                std::cout << "New connection with client " << client_socket << std::endl;

                int flags = fcntl(client_socket, F_GETFL, 0);
                fcntl(client_socket, F_SETFL, flags | O_NONBLOCK);

                event.events = EPOLLIN | EPOLLET;
                event.data.fd = client_socket;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event);
            }
            else if (events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {
                std::cout << "Connection closed with client file descriptor " << events[i].data.fd << std::endl;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                close(events[i].data.fd);
            }
            else if (events[i].events & EPOLLIN) {
                int bytes_read = recv(events[i].data.fd, buf, BUF_SIZE, 0);
                if (bytes_read == 0) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                    continue;
                }
                buf[bytes_read] = '\0'; 
                printf("%s\n", buf);
                send(events[i].data.fd, "HTTP/1.1 200 OK\n", 16, 0);
                send(events[i].data.fd, "Content-length: 46\n", 19, 0);
                send(events[i].data.fd, "Content-Type: text/html\n\n", 25, 0);
                send(events[i].data.fd, "<html><body><H1>Hello world</H1></body></html>", 46, 0);
            }
        }
    }

    free(buf);
}