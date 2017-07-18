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

int main(void) {
  struct sockaddr_storage their_addr;
  char buf[100];
  char s[INET6_ADDRSTRLEN];
  socklen_t addr_size;
  struct addrinfo hints, *res;
  int sockfd;
  int new_fd;
  int childId;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(NULL, "1337", &hints, &res);

  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

  bind(sockfd, res->ai_addr, res->ai_addrlen);

  listen(sockfd, 5);

  while (1) {
    int comms[2];
    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
    int numbytes = recv(new_fd, buf, 99, 0);
    buf[numbytes] = '\0';
    std::cout << "connection from "  << inet_ntop(their_addr.ss_family,((struct sockaddr_in*)(struct sockaddr *)&their_addr), s, sizeof s) << " (" << buf << ")\n";
    if (!fork) {
    } else {
      while (true) {
        std::string stdinput;
        getline(std::cin, stdinput);
        if (stdinput != "next"){
          send(new_fd, stdinput.c_str(), stdinput.size(), 0);
        } else {
          send(new_fd, "-q", 2, 0);
          break;
        }
        int numbytes = recv(new_fd, buf, 99, 0);
        buf[numbytes] = '\0';
        printf("%s",buf);
      }
    }
  }
}
