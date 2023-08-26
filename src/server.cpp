#include "constants.h"
#include "netinet/in.h"
#include "utils.h"
#include <arpa/inet.h>
#include <cassert>
#include <cstdint>
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

static int32_t parse_single_request(int conn_fd) {
    // 4-byte header
    char read_buf[4 + K_MAX_MSG + 1]{};
    errno = 0;
    int32_t err = read_full(conn_fd, read_buf, 4);
    if (err) {
        if (errno == 0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }

    uint32_t len{};
    memcpy(&len, read_buf, 4); // assume little endian
    if (len > K_MAX_MSG) {
        msg("request too long");
        return -1;
    }

    // request body
    err = read_full(conn_fd, &read_buf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    // do work
    read_buf[4 + len] = '\0';
    printf("client says: %s\n", &read_buf[4]);

    // reply using the same protocol
    const char reply[] = "world";
    char write_buf[4 + sizeof(reply)]{};

    len = (uint32_t)strlen(reply);
    memcpy(write_buf, &len, 4);
    memcpy(&write_buf[4], reply, len);

    return write_all(conn_fd, write_buf, 4 + len);
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
    while (true) {
        // accept
        struct sockaddr_in client_addr {};
        socklen_t socklen = sizeof(client_addr);
        int conn_fd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (conn_fd < 0) {
            continue; // error
        }

        while (true) {
            int32_t err = parse_single_request(conn_fd);
            if (err) {
                break;
            }
        }

        close(conn_fd);
    }
}
