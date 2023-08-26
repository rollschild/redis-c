#include "constants.h"
#include "utils.h"
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

static int32_t query(int fd, const char *text) {
    uint32_t len = (uint32_t)strlen(text);
    if (len > K_MAX_MSG) {
        return -1;
    }

    char write_buf[4 + K_MAX_MSG]{};
    memcpy(write_buf, &len, 4);
    memcpy(&write_buf[4], text, len);

    if (int32_t err = write_all(fd, write_buf, 4 + len)) {
        return err;
    }

    // 4 bytes header
    char read_buf[4 + K_MAX_MSG + 1];
    errno = 0;
    int32_t err = read_full(fd, read_buf, 4);

    if (err) {
        if (errno == 0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }

    memcpy(&len, read_buf, 4);
    if (len > K_MAX_MSG) {
        msg("too long");
        return -1;
    }

    // reply body
    err = read_full(fd, &read_buf[4], len);
    if (err) {
        msg("read() error");
        return err;
    }

    // do something
    read_buf[4 + len] = '\0';
    printf("server says: %s\n", &read_buf[4]);
    return 0;
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    // Internet socket address
    struct sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1

    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("connect");
    }

    // multiple requests...
    int32_t err = query(fd, "hello_1");
    if (err) {
        close(fd);
        return 0;
    }

    err = query(fd, "hello_2");
    if (err) {
        close(fd);
        return 0;
    }

    err = query(fd, "hello_3");
    if (err) {
        close(fd);
        return 0;
    }

    close(fd);
    return 0;
}
