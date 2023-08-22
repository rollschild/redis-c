#include "netinet/in.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

// AF_INET - IPv4
// AF_INET6 - IPv6 or dual-stack socket
// SOCK_STREAM - for TCP
int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;
    // configure socket
    // SO_REUSEADDR - bind to the same address if restarted
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // 0.0.0.0
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("bind()");
    }

    // listen
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen()");
    }

    while (1) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int conn_fd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (conn_fd < 0) {
            continue; // error
        }

        do_something(conn_fd);
        close(conn_fd);
    }
}
