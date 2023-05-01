#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket creation failed");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // change this to your server's IP address
    server_addr.sin_port = htons(8080);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connection failed");
        return 1;
    }

    char *message = "Hello, server!";
    if ((size_t)(send(sock, message, strlen(message), 0)) != strlen(message)) {
        perror("send failed");
        return 1;
    }

    close(sock);
    return 0;
}
