#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <zconf.h>
#include <csignal>
#include <string>
#include <iostream>

#define PORT 8889

void sighandler(int sig) {
    if (SIGINT == sig) {
        printf("ctrl+c pressed\n");
        exit(-1);
    } else if (SIGPIPE) {
        printf("receive SIGPIPE signal\n");
    }
}

void read_from_server(int client_fd) {
    // read server write
    char buff[1024] = {0};
    int readsize = 0;

    memset(buff, 0, sizeof(buff));

    readsize = read(client_fd, buff, sizeof(buff));
    if (-1 == readsize) {
        perror("read failed");
        exit(-1);
    } else if (0 == readsize) {
        printf("server quit(write side quit)\n");
        return;
    }

    printf("server say: %s", buff);
}

int main() {
    // 0.process sigint signal
    if (SIG_ERR != signal(SIGINT, sighandler)) {
        printf("signal SIGINT success\n");
    }

    if (SIG_ERR != signal(SIGPIPE, sighandler)) {
        printf("signal SIGPIPE success\n");
    }

    // 1.create socket
    int client_fd= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == client_fd) {
        perror("socket failed");
        exit(-1);
    }

    // port reuse
    int opt = 1;
    if (-1 == setsockopt(client_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt SO_REUSEPORT failed");
        exit(-1);
    }

    // 2.create sockaddr_in, set listen address and port
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    // os: small
    // network: big
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (-1 == connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        perror("connect failed");
        exit(-1);
    }

    read_from_server(client_fd);

    std::string s;
    while (1) {
        // client write to server
        std::cout << "to sever:";
        std::cin >> s;

        int writesize = write(client_fd, s.c_str(), s.length());
        printf("write size: %d\n", writesize);

        if (s.length() != writesize) {
            perror("write failed");

            if (errno == EPIPE) {
                printf("server quit, cannot write to server\n");
                exit(-1);
            } else {
                printf("link is busy\n");
            }
        }
    }

    return 0;
}
