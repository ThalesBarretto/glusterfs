# syncop framework
A coroutines-based, cooperative multi-tasking framework.

## Topics

- Glossary
- Lifecycle of a synctask
- Syncop wrappers and the fop signatures
- Existing usage


## Glossary

### syncenv

syncenv is an object that provides access to a pool of worker threads.
synctasks execute in a syncenv.

### synctask

synctask can be informally defined as a pair of function pointers, namely _the
call_ and _the callback_ (see syncop.h for more details).

        synctask_fn_t - 'the call'
        synctask_cbk_t - 'the callback'

synctask has two modes of operation,

1. The calling thread waits for the synctask to complete.
2. The calling thread schedules the synctask and continues.

synctask guarantees that the callback is called _after_ the call completes.

### Lifecycle of a synctask

A synctask could go into the following stages while in execution.

- CREATED  - On calling synctask_create/synctask_new.

- RUNNABLE - synctask is queued in env->runq.

- RUNNING  - When one of syncenv's worker threads calls synctask_switch_to.

- WAITING  - When a synctask calls synctask_yield.

- DONE     - When a synctask has run to completion.


                                +-------------------------------+
                                |            CREATED            |
                                +-------------------------------+
                                  |
                                  | synctask_new/synctask_create
                                  v
                                +-------------------------------+
                                |    RUNNABLE (in env->runq)    | <+
                                +-------------------------------+  |
                                  |                                |
                                  | synctask_switch_to             |
                                  v                                |
 +------+  on task completion   +-------------------------------+  |
 | DONE | <-------------------- |            RUNNING            |  | synctask_wake/wake
 +------+                       +-------------------------------+  |
                                  |                                |
                                  | synctask_yield/yield           |
                                  v                                |
                                +-------------------------------+  |
                                |    WAITING (in env->waitq)    | -+
                                +-------------------------------+

Note: A synctask is not guaranteed to run on the same thread throughout its
lifetime. Every time a synctask yields, it is possible for it to run on a
different thread.

## Syncop wrappers and the fop signatures

The `syncop_<fop>()` wrappers in `syncop.[ch]` are written by hand; they are
not generated from `libglusterfs/src/generator.py` like `defaults.c` is. Their
signatures follow the fop prototypes in `xlator.h` with these differences:

- `syncop_create`, `syncop_mkdir`, `syncop_mknod` and `syncop_symlink` take no
  `umask`; they wind the fop with `umask = 0`.
- `syncop_unlink` takes no `flags`; it winds the fop with `flags = 0`.
- Callback results come back through out-parameters: `xdata_out` on every
  wrapper except `syncop_seek`, and `struct iatt *` for the wrappers whose
  callbacks return attributes
  (`writev`, `readv`, `fsync`, `ftruncate`, `setattr`, `fsetattr`, `create`,
  `mkdir`, `mknod`, `symlink`, `link`). `syncop_truncate` is the
  exception: it reuses the `ftruncate` callback but does not expose the
  pre/post attributes.
- Five fops have no wrapper at all: `fentrylk`, `finodelk`, `getspec`,
  `namelink` and `rchecksum`.

When adding a fop that needs a wrapper, add it here by hand and keep the
argument order of the fop prototype.
