#include "constants.h"
#include "utils.h"
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

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

static int32_t send_req(int fd, const std::vector<std::string> &cmd) {
    uint32_t len = 4; // length of nstr itself, 4 bytes
    for (const std::string &s : cmd) {
        len += 4 + s.size(); // length of cmd + cmd itself
    }

    if (len > K_MAX_MSG) {
        return -1;
    }

    char wbuf[4 + K_MAX_MSG]{}; // length of the entire write buffer
    memcpy(&wbuf[0], &len, 4);  // nstr + all cmds
    uint32_t n = cmd.size();    // number of commands in the cmd vector
    memcpy(&wbuf[4], &n, 4);
    size_t curr_pos = 8;

    for (const std::string &s : cmd) {
        uint32_t sz = (uint32_t)s.size();
        memcpy(&wbuf[curr_pos], &sz, 4); // length of the current command
        memcpy(&wbuf[curr_pos + 4], s.data(), s.size());
        curr_pos += 4 + s.size();
    }

    return write_all(fd, wbuf, 4 + len);
}

static int32_t read_res(int fd) {
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

    uint32_t len{};
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

    uint32_t rescode{};
    if (len < 4) {
        msg("bad response");
        return -1;
    }
    memcpy(&rescode, &read_buf[4], 4);

    // do something
    read_buf[4 + len] = '\0';
    printf("server says: [%u] %.*s\n", rescode, len - 4, &read_buf[8]);
    return 0;
}

int main(int argc, char **argv) {
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

    std::vector<std::string> cmd{};
    for (int i = 1; i < argc; ++i) {
        cmd.push_back(argv[i]);
    }
    int32_t err = send_req(fd, cmd);
    if (err) {
        close(fd);
        return 0;
    }
    err = read_res(fd);
    if (err) {
        close(fd);
        return 0;
    }

    close(fd);
    return 0;
}
