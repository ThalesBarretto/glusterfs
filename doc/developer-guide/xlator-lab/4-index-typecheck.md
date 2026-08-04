# Step 04: Index Alignment and the Typecheck

> **Experiment:** E3 | **Fact log:** F7, F8, F9 | **Requires:** setup (README) | **Root:** no

## Objective

Show that the `_cbk` half of `struct xlator_fops` is a compile-time typecheck
device: `STACK_WIND_COMMON` (`stack.h:305`) contains
`typeof(fn##_cbk) tmp_cbk = rfn;`, so a wrongly-shaped callback is rejected
by the compiler.

Nothing executes; both targets use `-fsyntax-only`.

## Execution

### Step 1: The well-formed case

A lookup wind paired with a lookup-shaped callback:

```bash
cd 4-index-typecheck && make good
```

**Expected output:**

```text
GOOD: compiled clean
```

### Step 2: The mismatched case

The same wind paired with a writev-shaped, 8-argument callback:

```bash
make bad
```

**Expected output:**

```text
mismatch.c: error: initialization of 'fop_lookup_cbk_t'
  from incompatible pointer type ...
  stack.h:305:36: note: in definition of macro 'STACK_WIND_COMMON'
    305 |  typeof(fn##_cbk) tmp_cbk = rfn;
BAD: correctly rejected
```

The Makefile inverts the compiler's exit status with `!`, so
`make good && make bad` succeeds exactly when the compiler behaves as claimed.

## Analysis

Remove the `_cbk` half and this check could not exist - the members are the
"typechecking hack in STACK_WIND only" that `xlator.h:608` describes.

The same struct also carries the positional index: `get_fop_index_from_fn`
(`stack.h:236`) derives the FOP index from a member's offset in the struct.
Three invariants follow:

- Dispatch members stay ordered to match `GF_FOP`.
- The `_cbk` half comes strictly after the dispatch half.
- Three placeholder slots keep the alignment for forget, release, and
  releasedir, which live in `struct xlator_cbks`.

Counts and citations: fact log F7, F8. The stale `xlator.h:542` reference to
`glusterfs-fops.x` is F9; the companion comment-fix PR corrects it.
