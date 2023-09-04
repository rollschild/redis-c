# Simple Redis Implementation in C/C++

## Notes

### Sockets

- **server-side** vs. **client-side**
- Two kinds of mainstream sockets:
  - DARPA Internet Socket
  - Unix Socket

#### Internet Socket

- Two mainstream types:
  - Stream Sockets - `SOCK_STREAM`
    - reliable two-way connected communication streams
    - uses TCP
  - Datagram Sockets - `SOCK_DGRAM`
    - a.k.a. **connectionless socket**
    - uses UDP
-

#### Server-Side

- `socket()` - syscall that returns an **fd**
- `bind()` - associates an address to a socket fd
- `listen()` - enables user to accept connections to that address
- `accept()` - takes a listening fd
  - when a client makes a connection to the listening address,
  - `accept()` returns an fd that represents the connection socket
- `read()` - receives data from a TCP connection
- `write()` - sends data
- `close()` - destroys the resource referred by the fd and recycles fd
- `send()` and `recv()` might offer better control over data transmission than `read()` and `write()`

#### Client-Side

- `connect()`
  - takes a socket fd and address
  - makes a TCP connection to that address

### Protocol Parsing

- Used to spilt requests apart from the TCP byte stream
- Current scheme:

```txt
+-----+-----+-----+-----+----------
| len | msg1 | len | msg2 | more...
+-----+-----+-----+-----+----------
```

- Two parts:
  - 4-byte **little-endian** integer - length of the request
  - variable-length request

#### Protocol Desgin

##### Text vs. Binary

- Text
  - human-readable
  - HTTP
  - hard to parse - variable-length strings
- **Avoid unnecessary variable-length components**
- protocol parsing currently uses 2 `read()` syscalls
  - could use a **buffered I/O**

### Event Loop and Non-blocking I/O

- Three ways to handle concurrent connections in server-side network programming
  - **forking**
    - creates new **processes** for each client connection
  - **multi-threading**
    - uses **threads** instead of processes
  - **event loops**
    - polling
    - non-blocking I/O
- Use `poll` to determine which fd can be operated _without_ blocking
- when an I/O operation is done on an fd, it should be _non-blocking_
- In _blocking_ mode,
  - `read` blocks the caller when there are no data in the kernel
  - `write` blocks when the write buffer is full
  - `accept` blocks when there are no new connections in the kernel queue
- In **non-blocking** mode
  - operations either success without blocking, or
  - fail with errno `EAGAIN` - not ready
  - non-blocking operations that fail with `EAGAIN` _must be retried_ after the readiness was notified by `poll`
- `poll` is the _SOLE_ blocking operation in an **event loop**
- All blocking networking IO APIs (`read`, `write`, `accept`) have a _nonblocking_ mode
- APIs that do not have a non-blocking mode (`gethostbyname`, disk IOs) should be performed in **thread pools**
- Timers should also implemented within the event loop since we cannot `sleep` inside
- Also `select` and `epoll` on Linux
  - `epoll` consists of 3 syscalls:
    - `epoll_create`
    - `epoll_wait`
    - `epoll_ctl`
  - stateful API
  - used to manipulate an fd set create by `epoll_create`, which is operated upon by `epoll_wait`

#### Implementation

- Need buffers for reading/writing
  - in nonblocking mode, IO operations are often _deferred_
- `poll()`
  - `poll()` is actually _horribly_ slow for large number of connections
  - Use a dedicated event lib such as `libevent` instead
  - ask the OS to let us know when some data is ready to read on which sockets
- Plan of using `poll()`:
  - Keep an array of `struct pollfd`s with info about:
    - which socket descriptors should be monitored
    - what kind of events we want to monitor for
  - OS will _block_ on the `poll()` call _until_ one of the events occurs or user-specified timeout occurs
- For a request/response protocol, clients are _not_ limited to sending one request and waiting for the response at a time
  - could save some latency
  - \***\*pipelining\*\***

### `get`, `set`, `del`

- _command_: list of strings, such as `set key val`
- Scheme of the command:
  - `nstr` - number of strings - 4 bytes
  - `len` - length of the following string - 4 bytes

```txt
+------+-----+------+-----+------+-----+-----+------+
| nstr | len | str1 | len | str2 | ... | len | strn |
+------+-----+------+-----+------+-----+-----+------+
```

- Scheme of the response:
  - `res` - 4-byte status code
  - `data` - response string

```txt
+-----+---------
| res | data...
+-----+---------
```

-
