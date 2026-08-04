# Step 02: Registration

> **Experiment:** E1 | **Fact log:** F1, F5, F6 | **Requires:** setup (README), step 1 | **Root:** no

## Objective

Prove the loader resolves exactly one symbol, `xlator_api`, and refuses the
legacy by-name registration Translator 101 teaches.

The tutorial teaches it in so many words
(`doc/developer-guide/translator-development.md:19-23`):

> They need to have a very definite internal structure so that the
> translator-loading code can figure out where all the pieces are. The way it
> does this is to use dlsym to look for specific names within your
> shared-object file [...]

followed by the old `xlator.c` code looking up `"fops"`, `"cbks"`, and
`"init"` by name.

The test article, `legacy_only.c`, exports only `fops`/`cbks`/`init`/`fini`
and no `xlator_api` - exactly what the quoted passage instructs.

## Execution

### Step 1: Build

```bash
cd 2-registration && make
```

### Step 2a: Hermetic probe

The probe reproduces `xlator_dynload_apis()` (`xlator.c`, lines 262-278):
`dlopen`, then `dlsym(handle, "xlator_api")`.

```bash
W=$(git rev-parse --show-toplevel)
LD_LIBRARY_PATH=$W/libglusterfs/src/.libs \
  ./e1_loader_probe ../1-build-skeleton/anatomy_skel.so ./legacy_only.so
```

**Expected output:**

```text
../1-build-skeleton/anatomy_skel.so xlator_api=PRESENT  by-name[fops=y init=y]  => loader verdict: LOAD OK
./legacy_only.so xlator_api=ABSENT   by-name[fops=y init=y]  => loader verdict: HARD-FAIL (LG_MSG_DLSYM_ERROR, xlator.c:273-278)
```

### Step 2b: Real daemon

Deploy both modules, run the daemon on each volfile. Dynload happens during
graph construction, before any brick init - no root needed.

```bash
install -D legacy_only.so                       $XLATORDIR/testing/legacy-only.so
install -D ../1-build-skeleton/anatomy_skel.so $XLATORDIR/testing/anatomy-skel.so

LD_LIBRARY_PATH=$PREFIX/lib $PREFIX/sbin/glusterfsd -f legacy.vol -N -l /dev/stdout
LD_LIBRARY_PATH=$PREFIX/lib $PREFIX/sbin/glusterfsd -f skel.vol   -N -l /dev/stdout
```

**Expected output**, legacy-only - refused, graph construction fails:

```text
E [xlator.c:274:xlator_dynload_apis] 0-xlator: dlsym missing
  [{dlsym=.../testing/legacy-only.so: undefined symbol: xlator_api}]
```

**Expected output**, skeleton - the control loads; the later posix failure
just needs root and is irrelevant to the load test:

```text
I [anatomy_skel.c:41:init] 0-t-top: anatomy-skel loaded
E [MSGID: 113082] [posix-common.c:847:posix_init] 0-t-posix: /tmp/anatomy-brick: failed to set gfid [Operation not permitted]
```

## Analysis

The loader keys solely on `xlator_api`; the by-name globals are never
consulted as a fallback.

- The `xlator.h` promise of 4.x backward compatibility for the old exported
  fields describes a loader branch that does not exist.
- The mechanism was removed by "xlator: make 'xlator_api' mandatory"
  (2018-12-06), first shipped in 6.0.
- Released-source check: v5.0's `xlator.c` still carries the by-name
  fallback; v6.0's has none.
