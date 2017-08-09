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


std::string handleReceive(int insock) {
    char sizebuf[4];
    std::string receivedString;
    int numbytes = read(insock, sizebuf, 4);
    if (numbytes != -1) {
        int bytesToRecv = ((int*)sizebuf)[0];
        char buf[bytesToRecv];
        int bytesRemaining = bytesToRecv;
        while (bytesRemaining > 0) {
            int res = read(insock, buf + (bytesToRecv - bytesRemaining), bytesRemaining);
            if (res == -1) return "-1";
            bytesRemaining -= res;
        }
        buf[bytesToRecv] = '\0';
        receivedString = std::string(buf);
        return receivedString;
    } else {
        return "-1";
    }   
}

int handleSend(int insock) {
    std::string stdinput;
    std::cin >> stdinput;
    if (stdinput != "next") {
        int sendsize = (int)stdinput.size();
        send(insock, (char *)&sendsize, 4, 0);
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
            if (!handleSend(new_fd)) break;
            std::string received = handleReceive(new_fd);
            printf("%s\n", received.c_str());
            fflush(stdout);
        }
        std::cout << "Connection machine broke";
    }
}
