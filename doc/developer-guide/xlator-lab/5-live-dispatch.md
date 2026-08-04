# Step 05: Live Dispatch

> **Experiment:** E4 | **Fact log:** F14 | **Requires:** setup (README), steps 1-2 | **Root:** yes

## Objective

Observe on live I/O that the callback runs synchronously inside `STACK_WIND`,
before the FOP handler returns - Translator 101's enduring race lesson.

The graph is `anatomy-skel` over `storage/posix` (`2-registration/skel.vol`);
`trace.gdb` breaks on `skel_lookup` and on its callback.

Root is required: the posix brick sets `trusted.gfid` and the fuse mount is
system-wide.

## Execution

### Step 1: Start the daemon under gdb

Terminal 1:

```bash
sudo mkdir -p /tmp/anatomy-brick /tmp/mnt-e4
sudo env LD_LIBRARY_PATH=$PREFIX/lib gdb -q -batch -x 5-live-dispatch/trace.gdb \
  --args $PREFIX/sbin/glusterfs --debug -f 2-registration/skel.vol /tmp/mnt-e4
```

### Step 2: Drive real I/O

Terminal 2:

```bash
sudo sh -c 'echo hi > /tmp/mnt-e4/probe && cat /tmp/mnt-e4/probe'
ls -l /tmp/anatomy-brick/probe
```

**Expected output:**

```text
hi
-rw-r--r-- 2 root root 3 ... /tmp/anatomy-brick/probe
```

Link count 2 is the file plus its `.glusterfs` GFID handle hardlink.

### Step 3: Read the trace

Terminal 1 shows, per lookup (verbatim recorded run:
`5-live-dispatch/wind-unwind.log`):

**Expected output:**

```text
>>> WIND: skel_lookup entered (this=t-top); about to STACK_WIND lookup to FIRST_CHILD
>>> UNWIND: default_lookup_cbk fired (skel's cbk). Stack (note skel_lookup still below = cbk ran inside the wind):
#0 default_lookup_cbk   defaults.c:1546        <- skel's callback firing (UNWIND begins)
#1 posix_lookup         posix-entry-ops.c:391  <- the child served the FOP
#2 skel_lookup          anatomy_skel.c:25      <- our xlator, still on the stack
#3 default_lookup       defaults.c:3024        <- parent (meta) wound in via default
#4 meta_lookup          meta.c:46              <- auto-injected meta xlator
#5 syncop_lookup / fuse_first_lookup ...       <- FUSE entry
```

### Step 4: Tear down

```bash
sudo umount /tmp/mnt-e4
```

## Analysis

The callback at frame #0 executes while `skel_lookup` is still at frame #2:
it ran inside `STACK_WIND`. Do cleanup in the callback, never after the wind.

Two side observations:

- `meta` at frame #4 is auto-injected by `glusterfs_graph_prepare` - the
  reason the setup installs `meta.so`.
- Frame #3's `default_lookup` is meta's deliberate pass-through for ordinary
  inodes, not the `fill_defaults` back-fill; step 3 proves that separately.
