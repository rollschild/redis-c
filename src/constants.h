#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>

const size_t K_MAX_MSG = 4096;
const size_t K_MAX_ARGS = 1024;
const size_t K_RESIZING_WORK = 128;
const size_t K_MAX_LOAD_FACTOR = 8;
const size_t K_IDLE_TIMEOUT_MS = 5 * 1000;

enum {
    SER_NIL = 0, // NULL
    SER_ERR = 1, // Error code and message
    SER_STR = 2, // string
    SER_INT = 3, // int64
    SER_DBL = 4, // double
    SER_ARR = 5, // array
};

enum {
    ERR_UNKNOWN = 1,
    ERR_2BIG = 2,
    ERR_TYPE = 3,
    ERR_ARG = 4,
};

#endif /* CONSTANTS_H */
