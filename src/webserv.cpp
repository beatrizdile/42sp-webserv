
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE 500

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
    if (listen(socket_fd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    while (true)
    {
        int client_fd;
        if ((client_fd = accept(socket_fd, NULL, NULL)) < 0)
        {
            perror("accept");
            exit(1);
        }
        int bytes_read = recv(client_fd, buf, BUF_SIZE, 0);
        buf[bytes_read] = '\0'; 
        printf("%s\n", buf);
        send(client_fd, "HTTP/1.1 200 OK\n", 16, 0);
        send(client_fd, "Content-length: 46\n", 19, 0);
        send(client_fd, "Content-Type: text/html\n\n", 25, 0);
        send(client_fd, "<html><body><H1>Hello world</H1></body></html>", 46, 0);

        close(client_fd);
    }

    free(buf);
}