/*
 * MIT License
 *
 * Copyright (c) 2021 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef SC_SOCK_H
#define SC_SOCK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef SC_HAVE_CONFIG_H
    #include "config.h"
#else
    #define sc_sock_malloc  malloc
    #define sc_sock_realloc  realloc
    #define sc_sock_free  free
#endif

#if defined(_WIN32) || defined(_WIN64)
    #include <Ws2tcpip.h>
    #include <windows.h>
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")

typedef SOCKET sc_sock_int;

#else
    #include <sys/socket.h>

typedef int sc_sock_int;
#endif

#define SC_SOCK_BUF_SIZE 32768

enum sc_sock_rc
{
    SC_SOCK_WANT_READ = -4,
    SC_SOCK_WANT_WRITE = -2,
    SC_SOCK_ERROR = -1,
    SC_SOCK_OK = 0
};

enum sc_sock_ev
{
    SC_SOCK_NONE = 0u,
    SC_SOCK_READ = 1u,
    SC_SOCK_WRITE = 2u,
};

enum sc_sock_family
{
    SC_SOCK_INET = AF_INET,
    SC_SOCK_INET6 = AF_INET6,
    SC_SOCK_UNIX = AF_UNIX
};

struct sc_sock_fd
{
    sc_sock_int fd;
    enum sc_sock_ev op;
    int type; // user data
    int index;
};

struct sc_sock
{
    struct sc_sock_fd fdt;
    bool blocking;
    int family;
    char err[128];
};

/**
 * Initialize sock
 *
 * @param sock      sock
 * @param type      user data
 * @param blocking  is socket blocking
 * @param family    one of SC_SOCK_INET, SC_SOCK_INET6, SC_SOCK_UNIX
 */
void sc_sock_init(struct sc_sock *sock, int type, bool blocking, int family);

/**
 * Destroy sock
 *
 * @param sock sock
 * @return     '0' on success, negative number on failure.
 *              call sc_sock_error() for error string.
 */
int sc_sock_term(struct sc_sock *sock);

/**
 * @param sock sock
 * @param host host
 * @param port port
 * @return    '0' on success, negative number on failure.
 *             call sc_sock_error() for error string.
 */
int sc_sock_listen(struct sc_sock *sock, const char *host, const char *port);

/**
 * @param sock sock
 * @param in   sock struct pointer the incoming connection
 * @return    '0' on success, negative number on failure.
 *             call sc_sock_error() for error string.
 */
int sc_sock_accept(struct sc_sock *sock, struct sc_sock *in);

/**
 * @param sock         sock
 * @param dest_addr    destination addr
 * @param dest_port    destination port
 * @param source_addr  source addr (outgoing addr)
 * @param source_port  source port (outgoing port)
 * @return            '0' on success, negative number on failure.
 *                     call sc_sock_error() for error string.
 */
int sc_sock_connect(struct sc_sock *sock, const char *dest_addr,
                    const char *dest_port, const char *source_addr,
                    const char *source_port);

/**
 * Set socket blocking or nonblocking. Normally, you don't call this directly.
 * sc_sock_init() takes 'blocking' parameter, so sockets will be set according
 * to it.
 *
 * @param sock     sock
 * @param blocking blocking
 * @return         '0' on success, negative number on failure.
 *                 call sc_sock_error() for error string.
 */
int sc_sock_set_blocking(struct sc_sock *sock, bool blocking);

/**
 * @param sock sock
 * @param ms   timeout milliseconds
 * @return     '0' on success, negative number on failure.
 *             call sc_sock_error() for error string.
 */
int sc_sock_set_rcvtimeo(struct sc_sock *sock, int ms);

/**
 * @param sock sock
 * @param ms   timeout milliseconds
 * @return     '0' on success, negative number on failure.
 *             call sc_sock_error() for error string.
 */
int sc_sock_set_sndtimeo(struct sc_sock *sock, int ms);

/**
 * Finish connect for nonblocking connections. This function must be called
 * after sc_sock_poll() indicates socket is writable.
 *
 * @param sock sock
 * @return     '0' on success, negative number on failure.
 *             call sc_sock_error() for error string.
 */
int sc_sock_finish_connect(struct sc_sock *sock);

/**
 * @param sock  sock
 * @param buf   buf
 * @param len   len
 * @param flags normally should be zero, otherwise flags are passed to send().
 * @return      - on success, returns sent byte count.
 *              - SC_SOCK_WANT_WRITE on EAGAIN.
 *              - SC_SOCK_ERROR on error
 */
int sc_sock_send(struct sc_sock *sock, char *buf, int len, int flags);

/**
 * @param sock  sock
 * @param buf   buf
 * @param len   len
 * @param flags normally should be zero, otherwise flags are passed to recv().
 * @return      - on success, returns bytes received.
 *              - SC_SOCK_WANT_READ on EAGAIN.
 *              - SC_SOCK_ERROR on error
 */
int sc_sock_recv(struct sc_sock *sock, char *buf, int len, int flags);

/**
 * @param sock sock
 * @return     last error string
 */
const char *sc_sock_error(struct sc_sock *sock);

/**
 * @param sock sock
 * @param buf  buf
 * @param len  buf len
 * @return     local host:port string of the socket.
 */
const char *sc_sock_local_str(struct sc_sock *sock, char *buf, size_t len);

/**
 * @param sock sock
 * @param buf  buf
 * @param len  buf len
 * @return     remote host:port string of the socket.
 */
const char *sc_sock_remote_str(struct sc_sock *sock, char *buf, size_t len);

/**
 * Print socket in format "Local(127.0.0.1:8080), Remote(180.20.20.3:9000)"
 *
 * @param sock sock
 * @param buf  buf
 * @param len  buf len
 */
void sc_sock_print(struct sc_sock *sock, char *buf, size_t len);


struct sc_sock_pipe
{
    struct sc_sock_fd fdt;
    sc_sock_int fds[2];
    char err[128];
};

/**
 * Create pipe
 *
 * @param pipe pipe
 * @param type user data into struct sc_sock_fdt
 * @return '0' on success, negative number on failure,
 *         call sc_sock_pipe_err() to get error string
 */
int sc_sock_pipe_init(struct sc_sock_pipe *pipe, int type);

/**
 * Destroy pipe
 *
 * @param pipe pipe
 * @return '0' on success, negative number on failure,
 *         call sc_sock_pipe_err() to get error string
 */
int sc_sock_pipe_term(struct sc_sock_pipe *pipe);

/**
 * Write data to pipe
 *
 * @param pipe pipe
 * @param data data
 * @param len  data len
 * @return     written data len, normally pipe is blocking, return value should
 *             be equal to 'len'
 */
int sc_sock_pipe_write(struct sc_sock_pipe *pipe, void *data, unsigned int len);

/**
 * Read data from pipe
 *
 * @param pipe pipe
 * @param data destination
 * @param len  read size
 * @return     read data len, normally pipe is blocking, return value should
 *             be equal to 'len'
 */
int sc_sock_pipe_read(struct sc_sock_pipe *pipe, void *data, unsigned int len);

/**
 * Get error string
 * @param pipe pipe
 * @return     last error string
 */
const char *sc_sock_pipe_err(struct sc_sock_pipe *pipe);

#if defined(__linux__)

    #include <sys/epoll.h>

struct sc_sock_poll
{
    int fds;
    int count;
    int cap;
    struct epoll_event *events;
    char err[128];
};

#elif defined(__FreeBSD__) || defined(__APPLE__)
    #include <sys/event.h>

struct sc_sock_poll
{
    int fds;
    int count;
    int cap;
    struct kevent *events;
    char err[128];
};
#else

    #if !defined(_WIN32)
        #include <sys/poll.h>
    #endif

struct sc_sock_poll
{
    int count;
    int cap;
    void **data;
    struct pollfd *events;
    char err[128];
};

#endif

/**
 * Create poll
 *
 * @param poll poll
 * @return     '0' on success, negative number on failure,
 *             call sc_sock_poll_err() to get error string
 */
int sc_sock_poll_init(struct sc_sock_poll *poll);

/**
 * Destroy poll
 *
 * @param poll poll
 * @return     '0' on success, negative number on failure,
 *             call sc_sock_poll_err() to get error string
 */
int sc_sock_poll_term(struct sc_sock_poll *poll);

/**
 * Add fd to to poller.
 *
 * @param poll    poll
 * @param fdt     fdt
 * @param events  SC_SOCK_READ, SC_SOCK_WRITE or SC_SOCK_READ | SC_SOCK_WRITE
 * @param data    user data
 * @return        '0' on success, negative number on failure,
 *                call sc_sock_poll_err() to get error string
 */
int sc_sock_poll_add(struct sc_sock_poll *poll, struct sc_sock_fd *fdt,
                     enum sc_sock_ev events, void *data);

/**
 *
 * @param poll   poll
 * @param fdt    fdt
 * @param events SC_SOCK_READ, SC_SOCK_WRITE or SC_SOCK_READ | SC_SOCK_WRITE
 * @param data   user data
 * @return       '0' on success, negative number on failure,
 *               call sc_sock_poll_err() to get error string
 */
int sc_sock_poll_del(struct sc_sock_poll *poll, struct sc_sock_fd *fdt,
                     enum sc_sock_ev events, void *data);

/**
 * e.g
 *  int n = sc_sock_poll_wait(poll, 100);
 *  for (int i = 0; i < n; i++) {
 *      void *user_data = sc_sock_poll_data(poll, i);
 *      uint32_t events = sc_sock_poll_event(poll, i);
 *
 *      if (events & SC_SOCK_READ)  {
 *          // Handle read event
 *      }
 *
 *      if (events & SC_SOCK_WRITE)  {
 *          // Handle write event
 *      }
 *  }
 *
 * @param poll  poll
 * @param i
 * @return
 */
int sc_sock_poll_wait(struct sc_sock_poll *poll, int timeout);

/**
 *
 * @param poll poll
 * @param i    event index
 * @return     user data of fd at index 'i'
 */
void *sc_sock_poll_data(struct sc_sock_poll *poll, int i);

/**
 *
 * @param poll poll
 * @param i    event index
 * @return     events of fd at index 'i', events might be :
 *             - SC_SOCK_READ
 *             - SC_SOCK_WRITE
 *             - SC_SOCK_READ | SC_SOCK_WRITE
 *
 *             Closed fd will set SC_SOCK_READ | SC_SOCK_WRITE together. So,
 *             any attempt to read or write will indicate socket is closed.
 */
uint32_t sc_sock_poll_event(struct sc_sock_poll *poll, int i);

/**
 * Get error string
 *
 * @param poll poll
 * @return     last error string
 */
const char *sc_sock_poll_err(struct sc_sock_poll *poll);

#endif
