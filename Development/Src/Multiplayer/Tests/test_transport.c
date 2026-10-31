/* Unit test for ol_transport: two connections over a lossy, reordering, duplicating
   virtual link, plus random datagrams. Built and run by test_relay.py under ASan/UBSan. */
#include "ol_transport.h"
#include "third_party/monocypher/monocypher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(cond) do { if (!(cond)) { printf("FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond); exit(1); } } while (0)

typedef struct {
    uint8_t data[OL_MAX_DATAGRAM];
    int     len;
    double  arrives;
} Wire;

#define WIRE_MAX 200000
static Wire  *g_wire[2];
static int    g_wire_n[2];
static double g_now;
static int    g_loss, g_dup;
static double g_latency, g_jitter;

static int rnd(int n) { return rand() % n; }

static void link_send(void *user, const uint8_t *d, int len) {
    int to = (int)(size_t)user, copies = 1, i;
    CHECK(len <= OL_MAX_DATAGRAM);
    if (rnd(100) < g_loss) return;
    if (rnd(100) < g_dup) copies = 2;
    for (i = 0; i < copies; i++) {
        Wire *w;
        CHECK(g_wire_n[to] < WIRE_MAX);
        w = &g_wire[to][g_wire_n[to]++];
        memcpy(w->data, d, len);
        w->len = len;
        w->arrives = g_now + g_latency + g_jitter * rnd(1000) / 1000.0;
    }
}

/* The two ends of a connection, in opposite directions. */
static void key_pair(OlConn *c, int side) {
    uint8_t k0[32], k1[32];
    memset(k0, 0xA5, 32);
    memset(k1, 0x5A, 32);
    if (side) ol_conn_set_keys(c, k1, k0);
    else      ol_conn_set_keys(c, k0, k1);
}

typedef struct {
    int next_expected;
    int state_seen;
    int last_state;
} Rx;

static void deliver(void *user, int lane, uint8_t *msg, int len) {
    Rx *rx = (Rx *)user;
    int value;
    CHECK(len >= 4);
    memcpy(&value, msg, 4);
    if (lane == OL_LANE_RELIABLE) {
        CHECK(value == rx->next_expected);  /* exactly once, in order */
        rx->next_expected++;
    } else {
        CHECK(value > rx->last_state);      /* stale state must be dropped */
        rx->last_state = value;
        rx->state_seen++;
    }
}

static void pump(OlConn *conn, Rx *rx, int to) {
    int i, kept = 0;
    for (i = 0; i < g_wire_n[to]; i++) {
        Wire *w = &g_wire[to][i];
        if (w->arrives <= g_now)
            ol_conn_on_datagram(conn, g_now, w->data, w->len, deliver, rx);  /* 0 for a duplicate */
        else
            g_wire[to][kept++] = *w;
    }
    g_wire_n[to] = kept;
}

/* Each side sends `total` reliable messages of mixed sizes plus a state stream. */
static double run(int loss, int dup, double latency, double jitter, int total) {
    static OlConn a, b;
    Rx ra, rb;
    int sent_a = 0, sent_b = 0, state = 1;
    double started;

    g_loss = loss; g_dup = dup; g_latency = latency; g_jitter = jitter;
    g_wire_n[0] = g_wire_n[1] = 0;
    g_now = 1000.0;
    started = g_now;
    ol_conn_reset(&a, 42); key_pair(&a, 0);
    ol_conn_reset(&b, 42); key_pair(&b, 1);
    memset(&ra, 0, sizeof(ra));
    memset(&rb, 0, sizeof(rb));
    ra.last_state = rb.last_state = 0;

    while (ra.next_expected < total || rb.next_expected < total
           || ol_conn_reliable_pending(&a) || ol_conn_reliable_pending(&b)) {
        uint8_t msg[OL_MAX_MSG];
        int burst;
        memset(msg, 0x5A, sizeof(msg));
        for (burst = 0; burst < 8; burst++) {
            int len = 4 + rnd(rnd(10) == 0 ? OL_MAX_MSG - 4 : 300);
            if (sent_a < total && ol_conn_reliable_room(&a) > 0) {
                memcpy(msg, &sent_a, 4);
                CHECK(ol_conn_send(&a, OL_LANE_RELIABLE, msg, len));
                sent_a++;
            }
            if (sent_b < total && ol_conn_reliable_room(&b) > 0) {
                memcpy(msg, &sent_b, 4);
                CHECK(ol_conn_send(&b, OL_LANE_RELIABLE, msg, len));
                sent_b++;
            }
        }
        memcpy(msg, &state, 4);
        ol_conn_send(&a, OL_LANE_STATE, msg, 40);
        ol_conn_send(&b, OL_LANE_STATE, msg, 40);
        state++;

        ol_conn_flush(&a, g_now, link_send, (void *)(size_t)1);
        ol_conn_flush(&b, g_now, link_send, (void *)(size_t)0);
        g_now += 0.01;
        pump(&a, &ra, 0);
        pump(&b, &rb, 1);
        CHECK(g_now - started < 600.0);  /* must converge */
    }
    CHECK(ra.next_expected == total && rb.next_expected == total);
    if (loss < 50) CHECK(ra.state_seen > 0 && rb.state_seen > 0);
    ol_conn_free(&a);
    ol_conn_free(&b);
    return g_now - started;
}

static void test_limits(void) {
    static OlConn c;
    uint8_t big[OL_MAX_MSG + 1];
    int i;
    ol_conn_reset(&c, 1); key_pair(&c, 0);
    memset(big, 0, sizeof(big));
    CHECK(!ol_conn_send(&c, OL_LANE_RELIABLE, big, 0));
    CHECK(!ol_conn_send(&c, OL_LANE_RELIABLE, big, OL_MAX_MSG + 1));
    CHECK(ol_conn_send(&c, OL_LANE_RELIABLE, big, OL_MAX_MSG));
    CHECK(ol_conn_reliable_pending(&c) == 1);
    for (i = 1; i < OL_QUEUE_SIZE; i++) CHECK(ol_conn_send(&c, OL_LANE_RELIABLE, big, 4));
    CHECK(!ol_conn_send(&c, OL_LANE_RELIABLE, big, 4));
    CHECK(ol_conn_reliable_room(&c) == 0);
    ol_conn_free(&c);
}

static void test_wrong_conn_ignored(void) {
    static OlConn a, b;
    uint8_t d[OL_HEADER_BYTES];
    Rx rx;
    memset(&rx, 0, sizeof(rx));
    memset(d, 0, sizeof(d));
    d[0] = OL_MAGIC;
    d[1] = 99;
    ol_conn_reset(&a, 1); key_pair(&a, 0);
    ol_conn_reset(&b, 1); key_pair(&b, 1);
    CHECK(ol_conn_on_datagram(&a, 0.0, d, sizeof(d), deliver, &rx) == 0);
    CHECK(ol_conn_on_datagram(&a, 0.0, d, OL_HEADER_BYTES - 1, deliver, &rx) == 0);
    ol_conn_free(&a);
    ol_conn_free(&b);
}

static void ignore(void *user, int lane, uint8_t *msg, int len) {
    (void)user; (void)lane;
    if (len > 0) { volatile uint8_t x = msg[0]; x = msg[len - 1]; (void)x; }
}

static void test_fuzz(void) {
    static OlConn c;
    int i;
    ol_conn_reset(&c, 7); key_pair(&c, 0);
    for (i = 0; i < 300000; i++) {
        uint8_t d[OL_MAX_DATAGRAM], nonce[24], key[32];
        int len = OL_HEADER_BYTES + rnd(OL_MAX_DATAGRAM - OL_HEADER_BYTES + 1), j;
        for (j = 0; j < len; j++) d[j] = (uint8_t)rnd(256);
        d[0] = OL_MAGIC;
        memset(d + 1, 0, 8);
        d[1] = 7;
        /* Bias towards plausible framing so the parser goes deep. */
        if (rnd(2) && len > OL_HEADER_BYTES + 5) {
            d[OL_HEADER_BYTES] = (uint8_t)rnd(2);
            d[OL_HEADER_BYTES + 3] = (uint8_t)rnd(40);
            d[OL_HEADER_BYTES + 4] = 0;
        }
        /* Garbage that does not authenticate never reaches the parser. */
        CHECK(ol_conn_on_datagram(&c, i * 0.001, d, len, ignore, NULL) == 0);
        /* Authentic garbage does, with the key of the other end of key_pair(). */
        memset(key, 0x5A, 32);
        memset(nonce, 0, 24);
        memcpy(d + 9, &i, 4);
        memcpy(nonce, &i, 4);
        crypto_aead_lock(d + 29, d + 13, key, nonce, d, 13, d + 29, len - 29);
        CHECK(ol_conn_on_datagram(&c, i * 0.001, d, len, ignore, NULL) == 1);
        if (i % 16 == 0) {
            ol_conn_send(&c, OL_LANE_RELIABLE, d, 20);
            g_wire_n[0] = 0;
            ol_conn_flush(&c, i * 0.001, link_send, (void *)(size_t)0);
        }
    }
    ol_conn_free(&c);
}

int main(void) {
    double t;
    srand(1234);
    g_wire[0] = (Wire *)malloc(sizeof(Wire) * WIRE_MAX);
    g_wire[1] = (Wire *)malloc(sizeof(Wire) * WIRE_MAX);

    test_limits();
    test_wrong_conn_ignored();

    t = run(0, 0, 0.02, 0.0, 3000);
    printf("clean link:              %.2fs\n", t);
    t = run(5, 1, 0.1, 0.02, 3000);
    printf("5%% loss, 200 ms RTT:     %.2fs\n", t);
    t = run(20, 5, 0.05, 0.05, 2000);
    printf("20%% loss, reordering:    %.2fs\n", t);
    t = run(50, 10, 0.03, 0.05, 500);
    printf("50%% loss:                %.2fs\n", t);

    test_fuzz();
    free(g_wire[0]);
    free(g_wire[1]);
    printf("OK\n");
    return 0;
}
