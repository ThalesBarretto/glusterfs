# Step 03: Fill Defaults

> **Experiment:** E2 | **Fact log:** F4 | **Requires:** setup (README), steps 1-2 | **Root:** no

## Objective

Observe, pointer-exact, that `fill_defaults()` (`xlator.c:81`, called from
`xlator_dynload`) replaces every NULL FOP slot with the matching
`default_<fop>`.

The skeleton implements only `lookup`, and its `init` runs after dynload but
before the root-only posix brick init - so the finished FOP table can be
inspected under gdb with no mount and no root.

## Execution

### Step 1: Inspect the FOP table at init

From the lab root - step 2b already deployed the skeleton:

```bash
LD_LIBRARY_PATH=$PREFIX/lib gdb -q -batch \
  -ex 'set breakpoint pending on' \
  -ex 'break init if $_streq(this->name, "t-top")' \
  -ex run \
  -ex 'print this->fops->lookup == skel_lookup' \
  -ex 'print this->fops->stat   == default_stat' \
  -ex 'print this->fops->open   == default_open' \
  --args $PREFIX/sbin/glusterfsd -f 2-registration/skel.vol -N -l /dev/null
```

**Expected output:**

```text
Breakpoint 1, init (this=...) at anatomy_skel.c:33
$1 = 1
$2 = 1
$3 = 1
```

gdb stays attached to a live daemon afterwards; interrupt it when done.

## Analysis

The implemented slot holds `skel_lookup`; the slots never set hold
`default_stat` and `default_open`, installed by `fill_defaults`.

This is why an xlator implements only the FOPs it cares about, and why a
1-FOP skeleton - or the classic 2-FOP rot-13 - is a complete xlator.
