#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nanoev.h"
#include <assert.h>
#define ASSERT assert

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#else
# include <signal.h>
#endif

static nanoev_event *async_for_ctrl_c;
#ifdef _WIN32
static BOOL WINAPI CtrlCHandler(DWORD dwCtrlType)
{
    int ret;
    ASSERT(async_for_ctrl_c);
    ret = nanoev_async_send(async_for_ctrl_c);
    ASSERT(ret == NANOEV_SUCCESS);
    return TRUE;
}
#else
static void sigint_handler(int sig)
{
    int ret;
    ASSERT(async_for_ctrl_c);
    ret = nanoev_async_send(async_for_ctrl_c);
    ASSERT(ret == NANOEV_SUCCESS);
}
#endif

struct nanoev_addr local_addr;
struct nanoev_addr server_addr;
char read_buf[100];
int times = 0;
int max_times = 1000000;
const char *msg = "ABCD";
unsigned int msg_len = 4;
int read_success_count = 0;
int read_fail_count = 0;
int write_success_count = 0;
int write_fail_count = 0;

void on_read(
    nanoev_event *udp,
    int status,
    void *buf,
    unsigned int bytes,
    const struct nanoev_addr *from_addr
    )
{
    int ret_code;

    if (!status)
        ++read_success_count;
    else
        ++read_fail_count;

    //printf("on_read status=%d bytes=%u data=%s\n", status, bytes, buf);

    ret_code = nanoev_udp_read(udp, read_buf, sizeof(read_buf), on_read);
    ASSERT(ret_code == NANOEV_SUCCESS);
}

void on_write(
    nanoev_event *udp,
    int status,
    void *buf,
    unsigned int bytes
    )
{
    if (!status)
        ++write_success_count;
    else
        ++write_fail_count;

    //printf("on_write status=%d bytes=%u\n", status, bytes);
    
    ++times;
    if (times < max_times) {
        int ret_code = nanoev_udp_write(udp, msg, msg_len, &server_addr, on_write);
        ASSERT(ret_code == NANOEV_SUCCESS);
    } else {
        int ret_code = nanoev_async_send(async_for_ctrl_c);
        ASSERT(ret_code == NANOEV_SUCCESS);
    }
}

static void on_async(
    nanoev_event *async
    )
{
    nanoev_loop *loop = nanoev_event_loop(async);
    nanoev_loop_break(loop);
}

int main(int argc, char* argv[])
{
    int ret_code;
    nanoev_loop *loop;
    nanoev_event *async;
    nanoev_event *udp;
    nanoev_timeval tmStart, tmEnd;
    unsigned int duration;
    int family;
    const char *addr;
    unsigned short port;

    if (argc > 1 && strcmp(argv[1], "-ipv6") == 0) {
        family = NANOEV_AF_INET6;
        addr = "::1";
        port = 4000;
    } else {
        family = NANOEV_AF_INET;
        addr = "127.0.0.1";
        port = 4000;
    }

    ret_code = nanoev_init();
    ASSERT(ret_code == NANOEV_SUCCESS);
    loop = nanoev_loop_new(NULL);
    ASSERT(loop);

    async = nanoev_event_new(nanoev_event_async, loop, NULL);
    ASSERT(async);

    udp = nanoev_event_new(nanoev_event_udp, loop, NULL);
    ASSERT(udp);

    nanoev_addr_init(&local_addr, family, addr, port);
    ret_code = nanoev_udp_bind(udp, &local_addr);
    if (ret_code != NANOEV_SUCCESS) {
        printf("nanoev_udp_bind return %d, udp_error=%d\n", ret_code, nanoev_udp_error(udp));        
    }
    ASSERT(ret_code == NANOEV_SUCCESS);

    nanoev_now(&tmStart);

    ret_code = nanoev_udp_read(udp, read_buf, sizeof(read_buf), on_read);
    ASSERT(ret_code == NANOEV_SUCCESS);

    nanoev_addr_init(&server_addr, family, addr, port);
    ret_code = nanoev_udp_write(udp, msg, msg_len, &server_addr, on_write);
    ASSERT(ret_code == NANOEV_SUCCESS);

    ret_code = nanoev_async_start(async, on_async);
    ASSERT(ret_code == NANOEV_SUCCESS);
    async_for_ctrl_c = async;
#ifdef _WIN32
    SetConsoleCtrlHandler(CtrlCHandler, TRUE);
#else
    signal(SIGINT, sigint_handler);
#endif
    printf("Press Ctrl+C to break...\n");

    nanoev_loop_run(loop);

    nanoev_now(&tmEnd);
    duration = (tmEnd.tv_sec*1000 + tmEnd.tv_usec/1000) - (tmStart.tv_sec*1000 + tmStart.tv_usec/1000);
    printf("time = %u ms\n", duration);

    printf("read  success=%d fail=%d\n", read_success_count, read_fail_count);
    printf("write success=%d fail=%d\n", write_success_count, write_fail_count);

    nanoev_event_free(udp);

    nanoev_loop_free(loop);

    nanoev_term();

    return 0;
}
