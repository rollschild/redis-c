#include "netinet/in.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static void msg(const char *message) { fprintf(stderr, "%s\n", message); }

static void die(const char *message) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, message);
    abort();
}

static void do_something(int conn_fd) {
    /*
     * `read()` and `write()` returns the number of read/written bytes
     */
    char read_buf[64]{};
    ssize_t n = read(conn_fd, read_buf, sizeof(read_buf) - 1);

    if (n < 0) {
        msg("read() error");
        return;
    }

    printf("client says: %s\n", read_buf);

    char write_buf[] = "world";
    write(conn_fd, write_buf, strlen(write_buf));
}

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
    struct sockaddr_in addr {};
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

    // loop for each connection and do something
    while (1) {
        // accept
        struct sockaddr_in client_addr {};
        socklen_t socklen = sizeof(client_addr);
        int conn_fd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (conn_fd < 0) {
            continue; // error
        }

        do_something(conn_fd);
        close(conn_fd);
    }
}
