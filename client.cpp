#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "1337"

#define MAXDATASIZE 100

bool check(char* one, char* two, int len) {
    for (int i; i < len; ++i) {
	    if (one[i] != two[i]) {
			return false;	
	    }
    }
	return true;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1); }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); 

    while (true) {
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
    	}

		char isclose[3] = "-q";
		char isping[5] = "ping";
		char isexec[5] = "exec";

		buf[numbytes] = '\0';
		if (check(buf, isclose, 2) == true) {
			close(sockfd);
			return 0;
		} else if (check(buf, isping, 4) == true) {
			send(sockfd, "pong\n", 5, 0);	
		} else if (check(buf, isexec, 4)) {
			std::string cmd = "";
			for (int i = 5; buf[i] != '\0'; ++i) {
				cmd += buf[i];	
			}
			std::string out = "Executing command "+cmd;
			send(sockfd, out.c_str(), out.size(), 0);
		} else {
			std::string out1 = "";
			for (int i = 0; buf[i] != '\0'; ++i) {
				out1 += buf[i];
			}
			std::string out2 = "Unknown command: "+out1;
			send(sockfd, out2.c_str(), out2.size(), 0);
		}
    }
}
