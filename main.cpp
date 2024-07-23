#include <cstddef>
#include <cstdint>
#include <iostream>
#include "settings.h"
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>


#define HOST "httpbin.org"
#define PORT "80"

enum LOG_TYPE {
    INFO,
    ERROR
};

void log(LOG_TYPE lt, std::string message) {
    switch (lt) {
        case INFO:
            std::cout << "[INFO] ";
            break;
        case ERROR:
            std::cout << "[ERROR] ";
            break;
    }

    std::cout << message << std::endl;
}



std::string base_url = "https://httpbin.org";

int main() {
    
    struct addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo * addrs;
    if (getaddrinfo(HOST, PORT, &hints, &addrs) < 0) {
        log(ERROR, "Could not get address of `" HOST "` : ");
        std::cout << strerror(errno) << std::endl;
        exit(1);
    }

    int sd = 0;
    for (struct addrinfo * addr = addrs; addr != NULL; addr = addr->ai_next) {
        sd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        if (sd == -1) {
            break;
        }

        if (connect(sd, addr->ai_addr, addr->ai_addrlen) == 0) {
            break;
        }

        close(sd);
        sd = -1;
    }

    const char * req = "GET /get HTTP/1.1\r\nHost: " HOST"\r\naccept: application/json\r\n\r\n";

    size_t len = strlen(req);

    ssize_t bytes_written = write(sd, (const void *)req, len);
    if (bytes_written != (ssize_t)strlen(req)) {
        log(ERROR, "Error writing request: ");
        std::cout << strerror(errno) << std::endl;
        close(sd);
        return 1;
    }

    log(INFO, "Starts listening...");

    char buf[1024] = {0};
    ssize_t bytes_read = read(sd, buf, sizeof(buf));

    if (bytes_read > 0) {
        buf[bytes_read] = '\0';
        std::cout << "Response:\n" << buf << std::endl;
    } else {
        log(ERROR, "Error reading response: ");
       std::cout << strerror(errno) << std::endl;
    }

    close(sd);

    return 0;
}
