#!/bin/bash
#
# Regression test: a stale event-slot index must not abort the process.
#
# event-epoll.c looked the slot up from a caller-supplied idx and then
# assert()ed slot->fd == fd. Slots are recycled, so a stale idx from a
# connection that raced with its own unregistration addressed a slot now
# serving another fd, and the assert, live in release builds because NDEBUG
# is never defined, abort()ed the whole process: a brick, a FUSE client (and
# its mount), glusterd or a gfapi consumer. The tester recycles a slot
# through the public gf_event_* API and uses the stale index on all three
# affected entry points; no cluster required.
#
. $(dirname $0)/../include.rc

cleanup;

TESTER_SRC=$(dirname $0)/event-epoll-stale-idx.c
TESTER=$(dirname $0)/event-epoll-stale-idx

# gf_event_* live in libglusterfs, glfs_new() in libgfapi. -lgfapi makes
# build_tester pull the gluster libdir and includedir from pkg-config.
TEST build_tester $TESTER_SRC -lgfapi -lglusterfs
TEST $TESTER

cleanup_tester $TESTER

cleanup;
