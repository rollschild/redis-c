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
