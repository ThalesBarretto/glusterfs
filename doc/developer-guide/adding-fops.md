Adding a new FOP
================

The last FOP added was `copy_file_range` (commit 9fda5e5aac, 2018). It
touched every place listed below and is the precedent to follow.

1. `libglusterfs/src/glusterfs/glusterfs-fops.h`: add the `GF_FOP_*`
   constant. `libglusterfs/src/globals.c`: add its name to `gf_fop_list`.
2. Wire protocol: add `GFS3_OP_*` to `rpc/rpc-lib/src/protocol-common.h`,
   the request struct to `rpc/xdr/src/glusterfs4-xdr.x` (and a response
   struct, unless one of the shared `gfx_common_*_rsp` types fits), and the
   generated XDR symbols to `rpc/xdr/src/libgfxdr.sym`.
3. `libglusterfs/src/glusterfs/xlator.h`: add the `fop_<name>_t` and
   `fop_<name>_cbk_t` prototypes and both slots in `struct xlator_fops`: the
   fop itself and the `<name>_cbk` entry that `STACK_WIND` uses for type
   checking.
4. Default implementation. `libglusterfs/src/defaults.c` is generated at
   build time from the table in `libglusterfs/src/generator.py` and the
   template `libglusterfs/src/defaults-tmpl.c`:
    * `generator.py`: add an `ops['<name>']` entry listing the fop arguments
      (`fop-arg` rows) and the callback arguments (`cbk-arg` rows), each as
      `(role, name, C type)`. The generator derives `default_<name>`,
      `default_<name>_cbk`, `default_<name>_resume`,
      `default_<name>_cbk_resume` and `default_<name>_failure_cbk` from it.
    * `defaults-tmpl.c`: add the `.<name> = default_<name>` line to the
      `_default_fops` initializer. This table is not generated.
    * `libglusterfs/src/glusterfs/defaults.h`: declare the five functions.
    * `libglusterfs/src/xlator.c`: add `SET_DEFAULT_FOP(<name>)` to
      `fill_defaults()`.
    * `libglusterfs/src/libglusterfs.sym`: export the new symbols.
5. `libglusterfs/src/call-stub.[ch]` and `libglusterfs/src/default-args.[ch]`:
   add the stub and the argument store/wipe functions. These are written by
   hand; the generator does not cover them.
6. `libglusterfs/src/syncop.[ch]`: add a `syncop_<name>()` wrapper if the fop
   will be called synchronously. See `syncop.md` for how the wrappers differ
   from the fop prototypes.
7. `xlators/protocol/client` and `xlators/protocol/server`: add the fop to
   `client-rpc-fops_v2.c` and `server-rpc-fops_v2.c`, with the matching
   helpers in `client-common.c`, `server-common.c` and `server-resolve.c`.
8. Entry points: `xlators/mount/fuse/src/fuse-bridge.c` for FUSE and
   `api/src/glfs-fops.c`, `api/src/glfs.h`, `api/src/gfapi.map` and
   `api/src/gfapi.aliases` for gfapi, as applicable.
9. Implement the fop in `xlators/storage/posix` and in every translator for
   which the default pass-through is not sufficient. Translators that keep
   their own list of fops need an entry even when they only pass the call
   through: `xlators/debug/io-stats` and the generated fops of
   `xlators/features/utime` (`utime-gen-fops-c.py`, `utime-gen-fops-h.py`)
   are the known cases.
10. Add a test. `tests/basic/gfapi/gfapi-copy-file-range.t` with
    `glfs-copy-file-range.c` is the precedent for a gfapi-driven test.
