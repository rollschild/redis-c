#ifndef UTILS_H
#define UTILS_H

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
static void msg(const char *message) { fprintf(stderr, "%s\n", message); }

static void die(const char *message) {
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, message);
    abort();
}

static int32_t write_all(int fd, const char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0) {
            return -1; // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;

        buf += rv;
    }
    return 0;
}

static int32_t read_full(int fd, char *buf, size_t n) {
    while (n > 0) {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0) {
            return -1; // error or unexpected EOF
        }
        assert((size_t)rv <= n);

        n -= (size_t)rv;
        // advance the buf pointer by rv positions
        buf += rv;
    }
    return 0;
}

#endif /* UTILS_H */
