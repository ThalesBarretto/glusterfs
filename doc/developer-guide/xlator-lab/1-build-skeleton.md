# Step 01: Build the Skeleton

> **Experiment:** E0 | **Fact log:** F6, F14 | **Requires:** setup (README) | **Root:** no

## Objective

Build `anatomy_skel.so`, a complete one-FOP xlator, standalone - with the
recipe that works today, because the classic Translator 101 recipe no longer
compiles.

## Execution

### Step 1: Build

```bash
cd 1-build-skeleton && make
```

One `cc` line produces `anatomy_skel.so`. `GLFS_SRC` defaults to the
enclosing repository; override it for an external tree.

### Step 2: Inspect the symbol surface

```bash
nm -D anatomy_skel.so | grep -E 'xlator_api|fops|cbks|init|fini'
```

**Expected output:**

```text
0000000000004500 B cbks
0000000000001649 T fini
00000000000040e0 D fops
000000000000155c T init
0000000000004480 D xlator_api
```

## Analysis

A valid xlator is one `.c` file exporting one `xlator_api_t`.

The classic recipe fails on the first header; what it is missing is exactly
what this Makefile adds, with flags distilled from the tree's own
`compile_commands.json` rather than guessed:

- `-include config.h -include site.h` - the glusterfs headers use autoconf
  macros like `SIZEOF_LONG` but do not include `config.h` themselves.
- `-I rpc/xdr/src`, `-I/usr/include/uuid`, `-I/usr/include/tirpc` - include
  paths the classic recipe predates.

One nuance in the symbol listing: the by-name globals (`fops`, `cbks`, ...)
still appear - `xlator_api.fops = &fops` references them - but the loader
reads only `xlator_api`. Step 2 proves that.
