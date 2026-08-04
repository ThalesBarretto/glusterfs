# Xlator Anatomy Lab

A step-by-step, re-runnable reproducer: the experiment series behind the
translator-documentation issue filed against gluster/glusterfs. Do the setup
below, then walk the steps in order. Each step states its objective, gives
commands with expected output, and closes with analysis. Re-run at any commit
to re-verify the claims mechanically.

Recorded at devel `ae1d69672f`, version string `12dev`. File and line
citations resolve at that commit.

## Contents

| Step | Topic | Requires |
|------|-------|----------|
| [1](1-build-skeleton.md) | Build a minimal modern xlator | setup |
| [2](2-registration.md) | Registration: the loader reads one symbol | setup, 1 |
| [3](3-fill-defaults.md) | Fill defaults: NULL FOP slots are back-filled | setup, 1-2 |
| [4](4-index-typecheck.md) | Index alignment and the STACK_WIND typecheck | setup |
| [5](5-live-dispatch.md) | Live dispatch: the callback fires inside the wind | setup, 1-2, **root** |
| [appendix](appendix-fact-log.md) | The source-verification fact log | - |

Each step directory holds that step's sources, Makefiles, volfiles, and
captured output. The fact log and some source comments use the original
experiment ids; they map to steps as E0->1, E1->2, E2->3, E3->4, E4->5.

## Setup

Every step compiles against a configured and built glusterfs source tree -
this repository - and the daemon steps (2, 3, 5) also run binaries from a
`--prefix` install of it. Root is required only in step 5.

Build the tree, from the repository root:

```bash
./autogen.sh
./configure --prefix=$HOME/glfs-prefix
make -j"$(nproc)"
```

Install into the prefix:

```bash
make install
```

An unprivileged `make install` aborts at the `/sbin/mount.glusterfs` step with
`Permission denied`. That is expected; everything before it is already
installed, but the abort skips two pieces the lab needs. Install them
explicitly:

```bash
make -C glusterfsd install       # daemon binaries
make -C xlators/meta install     # auto-loaded by the fuse client (step 5)
```

Export the variables the steps use, and create the brick directory:

```bash
export PREFIX=$HOME/glfs-prefix
export XLATORDIR=$(ls -d $PREFIX/lib/glusterfs/*/xlator)
mkdir -p /tmp/anatomy-brick
```

Check: `$PREFIX/sbin/glusterfsd --version` prints the built version, and
`$XLATORDIR` contains `storage/`, `mount/`, and `meta.so`.

## Notes

- This branch carries the lab only; "Part I"/"Part II" in older prose refer to
  the companion reference and tutorial (the proposed
  `translator-development.md` replacement), submitted separately. In-tree
  placement of either is the maintainers' call.
- `5-live-dispatch/wind-unwind.log` is a verbatim capture from the recorded
  run.
- The full series was re-executed from this in-tree layout on 2026-08-04 at
  the anchor; all results reproduced.
