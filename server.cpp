#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

int connectionSetup(int insock) {
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, "1337", &hints, &res);

    insock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    bind(insock, res->ai_addr, res->ai_addrlen);

    listen(insock, 5);

    return insock;

}


int handleReceive(int insock) {
    char buf[100];
    int recvBytes = recv(insock, buf, 99, 0);
    if (recvBytes == -1 || recvBytes == 0) {
        close(insock);
        return 1;
    } else {
        printf("%s",buf);
        return 0;
    }
}

int handleSend(int insock) {
    std::string stdinput;
    std::cin >> stdinput;
    if (stdinput != "next") {
        send(insock, stdinput.c_str(), stdinput.size(), 0);
        return 0;
    } else {
        close(insock);
        return 1;
    }
}

int main(void) {
    struct sockaddr_storage their_addr;
    char s[INET6_ADDRSTRLEN];
    socklen_t addr_size;
    int sockfd;
    int new_fd;
    int childId;

    sockfd = connectionSetup(sockfd);

    while (1) {
        addr_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
        std::cout << "Connection from " << inet_ntop(their_addr.ss_family,((struct sockaddr_in*)(struct sockaddr *)&their_addr), s, sizeof s) << '\n';
        while(1) {
            if (!handleReceive(new_fd)) break;
            if (!handleSend(new_fd)) break;
        }
        std::cout << "Connection machine broke";
    }
}
