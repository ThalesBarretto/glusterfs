/*
 * Regression tester: a stale event-slot index must not abort the process.
 *
 * event-epoll.c looks a slot up from a caller-supplied idx and used to
 * assert(slot->fd == fd) on the result. Slots are pooled and recycled, so an
 * idx that outlived its registration addresses a slot that now serves
 * another fd, and the assert (live in release builds, NDEBUG is never
 * defined) abort()ed the whole process. This tester recycles a slot through
 * the public gf_event_* API and then uses the stale index on each affected
 * entry point: the library must return -1 and stay usable.
 *
 * Only public headers: glfs_new() attaches a ctx to the global xlator so the
 * library's log path works; the event-pool API is declared here because its
 * header is not consumable outside the tree.
 */
#include <glusterfs/api/glfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

struct event_pool;
typedef void (*event_handler_t)(int fd, int idx, int gen, void *data,
                                int poll_in, int poll_out, int poll_err,
                                int event_thread_exit);
struct event_pool *
gf_event_pool_new(int count, int eventthreadcount);
int
gf_event_register(struct event_pool *event_pool, int fd,
                  event_handler_t handler, void *data, int poll_in,
                  int poll_out, int notify_poller_death);
int
gf_event_unregister(struct event_pool *event_pool, int fd, int idx);
int
gf_event_select_on(struct event_pool *event_pool, int fd, int idx, int poll_in,
                   int poll_out);
int
gf_event_handled(struct event_pool *event_pool, int fd, int idx, int gen);

static void
handler(int fd, int idx, int gen, void *data, int poll_in, int poll_out,
        int poll_err, int event_thread_exit)
{
}

static int
check(const char *what, int ret, int expect)
{
    printf("%-44s rc=%d expected=%d %s\n", what, ret, expect,
           ret == expect ? "ok" : "FAIL");
    return ret == expect ? 0 : 1;
}

int
main(void)
{
    glfs_t *fs = NULL;
    struct event_pool *pool = NULL;
    int a[2], b[2], idx_a, idx_b, fails = 0;

    fs = glfs_new("event-epoll-stale-idx");
    if (!fs) {
        perror("glfs_new");
        return 2;
    }
    pool = gf_event_pool_new(1024, 1);
    if (!pool) {
        perror("gf_event_pool_new");
        return 2;
    }
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, a) ||
        socketpair(AF_UNIX, SOCK_STREAM, 0, b)) {
        perror("socketpair");
        return 2;
    }

    /* recycle a slot: register A, unregister A, register B */
    idx_a = gf_event_register(pool, a[0], handler, NULL, 1, 0, 0);
    fails += check("register A", idx_a < 0 ? -1 : 0, 0);
    fails += check("unregister A (slot freed)",
                   gf_event_unregister(pool, a[0], idx_a), 0);
    idx_b = gf_event_register(pool, b[0], handler, NULL, 1, 0, 0);
    fails += check("register B", idx_b < 0 ? -1 : 0, 0);
    if (idx_b != idx_a) {
        printf(
            "slot was not recycled (A idx=%d, B idx=%d): cannot exercise "
            "the stale path\n",
            idx_a, idx_b);
        return 1;
    }

    /* A's old idx now addresses B's slot: each call must fail, not abort */
    fails += check("stale gf_event_unregister",
                   gf_event_unregister(pool, a[0], idx_a), -1);
    fails += check("stale gf_event_select_on",
                   gf_event_select_on(pool, a[0], idx_a, 1, 1), -1);
    fails += check("stale gf_event_handled",
                   gf_event_handled(pool, a[0], idx_a, 0), -1);

    /* and the pool is still sane afterwards */
    fails += check("unregister B with its real idx",
                   gf_event_unregister(pool, b[0], idx_b), 0);

    /* the slot is now free (nobody re-registered it): the stale index
     * must still fail, not abort, and must not disturb the free slot */
    fails += check("stale gf_event_unregister on a free slot",
                   gf_event_unregister(pool, a[0], idx_a), -1);
    fails += check("stale gf_event_select_on on a free slot",
                   gf_event_select_on(pool, a[0], idx_a, 1, 1), -1);
    fails += check("stale gf_event_handled on a free slot",
                   gf_event_handled(pool, a[0], idx_a, 0), -1);
    fails += check(
        "register C (pool reusable)",
        gf_event_register(pool, a[1], handler, NULL, 1, 0, 0) < 0 ? -1 : 0, 0);
    fails += check(
        "register D (second free slot found)",
        gf_event_register(pool, b[1], handler, NULL, 1, 0, 0) < 0 ? -1 : 0, 0);

    return fails ? 1 : 0;
}
