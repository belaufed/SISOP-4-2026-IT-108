#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    int sock;
    struct sockaddr_in server;
    char message[1000], server_reply[2000];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(9000);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    while (1) {
        printf("db > ");
        fgets(message, 1000, stdin);

        send(sock, message, strlen(message), 0);

        if (strncmp(message, "EXIT", 4) == 0)
            break;

        int read_size = recv(sock, server_reply, 2000, 0);

        server_reply[read_size] = '\0';

        printf("%s\n", server_reply);
    }

    close(sock);

    return 0;
}
