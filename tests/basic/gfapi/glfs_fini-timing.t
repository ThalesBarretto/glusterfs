#!/bin/bash
#
# glfs_fini() of a libgfapi client that has done I/O must complete well within
# the 10 s call_bail period. Regression guard for the teardown-time timer-cancel
# stall (#4320): when gf_timer_call_cancel() refuses to cancel during ctx cleanup
# and the caller releases the timer's rpc_clnt ref only on a successful cancel,
# glfs_fini() waits for the bail event to fire (~11 s measured). The fixed path
# takes ~0.1-1.1 s (the pool-drain countdown); 5 s splits the two by a wide margin.

. $(dirname $0)/../../include.rc
. $(dirname $0)/../../volume.rc

cleanup;

TEST glusterd

TEST $CLI volume create $V0 $H0:$B0/brick1;
EXPECT 'Created' volinfo_field $V0 'Status';

TEST $CLI volume start $V0;
EXPECT 'Started' volinfo_field $V0 'Status';

logdir=`gluster --print-logdir`

TEST build_tester $(dirname $0)/glfs_fini-timing.c -lgfapi

ms=$(./$(dirname $0)/glfs_fini-timing $H0 $V0 $logdir/glfs_fini-timing.log)
echo "# glfs_fini took ${ms:-?} ms"
TEST [ -n "$ms" ]
TEST [ "$ms" -lt 5000 ]

cleanup_tester $(dirname $0)/glfs_fini-timing

TEST $CLI volume stop $V0
TEST $CLI volume delete $V0

cleanup;
