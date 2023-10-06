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

static int32_t on_response(const uint8_t *data, size_t size) {
    if (size < 1) {
        msg("bad response");
        return -1;
    }

    switch (data[0]) {
    case SER_NIL:
        printf("(nil)\n");
        return 1;
    case SER_ERR:
        if (size < 1 + 8) {
            msg("bad response");
            return -1;
        }
        {
            int32_t code = 0;
            uint32_t len = 0;
            memcpy(&code, &data[1], 4);
            memcpy(&len, &data[1 + 4], 4);
            if (size < 1 + 8 + len) {
                msg("bad response");
                return -1;
            }
            printf("(err) %d %.*s\n", code, len, &data[1 + 8]);
            return 1 + 8 + len;
        }
    case SER_STR:
        if (size < 1 + 4) {
            msg("bad response");
            return -1;
        }
        {
            uint32_t len = 0;
            memcpy(&len, &data[1], 4);
            if (size < 1 + 4 + len) {
                msg("bad response");
                return -1;
            }
            printf("(str) %.*s\n", len, &data[1 + 4]);
            return 1 + 4 + len;
        }
    case SER_INT:
        if (size < 1 + 8) {
            msg("bad response");
            return -1;
        }
        {
            int64_t val = 0;
            memcpy(&val, &data[1], 8);
            printf("(int) %ld\n", val);
            return 1 + 8;
        }
    case SER_DBL:
        if (size < 1 + 8) {
            msg("bad response");
            return -1;
        }
        {
            double val = 0;
            memcpy(&val, &data[1], 8);
            // `%g`, floating point, but without trailing zeros (and trailing
            // decimal point if fraction part is all zero)
            printf("(dbl) %g\n", val);
            return 1 + 8;
        }
    case SER_ARR:
        if (size < 1 + 4) {
            msg("bad response");
            return -1;
        }
        {
            uint32_t len = 0;
            memcpy(&len, &data[1], 4);
            printf("(arr) len=%u\n", len);
            size_t arr_bytes = 1 + 4;

            for (uint32_t i = 0; i < len; ++i) {
                int32_t rv = on_response(&data[arr_bytes], size - arr_bytes);
                if (rv < 0) {
                    return rv;
                }
                arr_bytes += (size_t)rv;
            }

            printf("(arr) end\n");
            return (int32_t)arr_bytes;
        }
    default:
        msg("bad response");
        return -1;
    }
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

    int32_t rv = on_response((uint8_t *)&read_buf[4], len);
    if (rv > 0 && (uint32_t)rv != len) {
        msg("bad response");
        rv = -1;
    }
    return rv;
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
