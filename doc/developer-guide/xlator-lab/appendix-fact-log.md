# Appendix: Fact Log

> Source-verification record behind the steps. Experiment ids map to steps:
> E0->1, E1->2, E2->3, E3->4, E4->5.

**Anchor SHA:** `ae1d69672fe0271265ab6f09ec2c2f93d4208511`
(glusterfs `devel`: *"Fix: use unsigned value to represent bytes (#4493)"*)

Every citation below resolves at the anchor SHA; paths are relative to the
repository root. Every fact is source-verified (read directly in the cited
file); the last column names the step that additionally confirms it
experimentally.

| # | Claim | Citation | Confirmation | Confirmed in |
|---|-------|----------|--------------|--------------|
| F1 | Load path `xlator_set_type`->`xlator_dynload`->`xlator_dynload_apis`; latter `dlsym(handle,"xlator_api")` and **hard-fails** (`ret=-1; goto out`) if absent; **no by-name fallback** | `libglusterfs/src/xlator.c:415, 374, 262, 272-278` | Read loader bodies end-to-end | step 2 |
| F2 | `.fops` mandatory (warn `LG_MSG_STRUCT_MISS` + fail if NULL); `.cbks`/`.dumpops` optional; pulled from the struct | `xlator.c:280-285, 287-290` | Read | step 2 |
| F3 | `dlsym(handle,"options")` is the **rpc-transport** branch (`socket.so`, not an xlator), not legacy compat | `xlator.c:202-206, 237-246` | Read | - |
| F4 | `fill_defaults(xl)` runs after api load -> unset FOP slots back-filled from `defaults.c` | `xlator.c:405` (call), `:81` (def) | Read | step 3 |
| F5 | **STALE COMMENT:** "old exported fields will be supported" - contradicted by F1 | `xlator.h:845-847` | Read; contradicted by loader | step 2 |
| F6 | Translator 101 teaches `dlsym(handle,"fops"/"cbks"/"init"/"fini")` by-name - **removed in GlusterFS v6.0** by "xlator: make 'xlator_api' mandatory" (Amar Tumballi, 2018-12-06; release-line `af7e957b49`, current-devel twin `67bbd5471a`, identical patch-id `571f8c9e16...`); `xlator_api_t` itself introduced 2017-11-28 | `translator-development.md:25-49`; v5.0<->v6.0 `xlator.c` | git-archaeology: v5.0 has 4 by-name `dlsym`, v6.0 has 0; commit diff removes fops/cbks/class_methods/init/fini/reconfigure/notify/dumpops/mem_acct_init; twin confirmed by patch-id | step 2 |
| F7 | `GF_FOP_MAXVALUE = 0 + 59` = **enum span** (NULL + 58 named ops, `STAT=1`...`COPY_FILE_RANGE=58`); of the 58, **55 dispatchable FOPs** + 3 cbk-only placeholders (`FORGET=41`/`RELEASE=42`/`RELEASEDIR=43`); classic-doc "82" superseded | `glusterfs-fops.h:76, 17, 58-60, 75`; `xlator.h:545-672`; `translator-development.md:85` | 59 enumerators; 55 `fop_*_t` + 3 placeholders counted | step 4 |
| F8 | `struct xlator_fops` = **116** fields: 55 `fop_*_t` + 3 `*_placeholder` (dispatch half = 58) mirrored by 55 `fop_*_cbk_t` + 3 `*_placeholder_cbk` (58) ("typechecking hack in STACK_WIND _only_"... "relative position is used to get the index"); positionally index-aligned to `GF_FOP` (`get_fop_index_from_fn` adds `+1` since `GF_FOP_NULL`=0 has no slot) | `xlator.h:545, 608-610, 652-655, 672`; `stack.h:236` | Re-counted via field-extraction script: **116** (55+3+55+3). **Correction: the prior "110" (and "59 FOPs") was wrong - see history footer.** | step 4 |
| F9 | **STALE PATH:** xlator.h:542-544 says order must match `rpc/xdr/src/glusterfs-fops.x` - file does not exist. Current wire/proc numbering = `enum gf_fop_procnum` in `protocol-common.h:16` (`GFS3_OP_WRITE` :30) | `xlator.h:542-544` (find: absent); `rpc/rpc-lib/src/protocol-common.h:16,30` | `find` empty; enum located | - |
| F10 | `struct xlator_cbks` = **11** members | `xlator.h:691-703` | Counted: 11 | - |
| F11 | `struct xlator_dumpops` = **11** members; only NULL-checked/usually-absent table | `xlator.h:730-742` | Counted: 11 | - |
| F12 | `xlator_api_t` GD2MARKER ABI freeze: first 4 fields (`op_version`,`identifier`,`options`,`category`) read by GD2, must not reorder | `xlator.h:852-919, 869-874` | Read | - |
| F13 | `xlator_t`/`struct _xlator` runtime instance (dlhandle, fops/cbks/dumpops, private, children/parents, xl_id, itable, ctx, graph, pass_through[_fops], category, ...) | `xlator.h:749-843` | Read | - |
| F14 | Worked examples present: `xlators/playground/template/`, `xlators/playground/rot-13/` | `ls` | Both present | steps 1, 5 |

## Resolved deferred items

- **`glusterfs-fops.x` absence** - confirmed via `find` (no match tree-wide). The ordering invariant
  is real but its named reference file is gone; the live FOP/proc numbering authority is
  `enum gf_fop_procnum` (`protocol-common.h`) and `GF_FOP_*` (`glusterfs-fops.h`).
- **`protocol-common.h` location** - confirmed: `enum gf_fop_procnum` at
  `protocol-common.h:16`.

## Status

All 14 facts source-verified at the anchor SHA. **F7 and F8 were revised post-hoc** (count
correction - see History). Facts F1, F2, F4, F5, F6, F8, F14 carry experimental obligations (E1-E4) -
they graduate to `experimentally-confirmed` when those experiments run. All
have since run - see the step files.

## History

- **2026-06-27 - FOP-count correction (F7, F8).** During the diagram-review pass the `struct
  xlator_fops` field count was re-derived with a field-extraction script (not by-eye), giving **116**
  fields (55 `fop_*_t` + 3 `*_placeholder` + 55 `fop_*_cbk_t` + 3 `*_placeholder_cbk`), not the
  originally-logged **110**. The "59" headline was also sharpened: `GF_FOP_MAXVALUE = 59` is the enum
  span (NULL + 58 named ops); only **55** of those are dispatchable FOPs, 3 are cbk-only placeholders.
  The 110 was a by-eye miscount carrying an `[SV]` tag it had not earned - the kind of error a build-
  and-count step catches that read-and-reason does not. README sec. 2.2/sec. 8/sec. 9 and the anatomy diagram were
  corrected to match.
- **2026-06-27 - by-name-removal provenance (F6).** Pinned *when* the legacy by-name registration was
  removed: **GlusterFS v6.0**, by "xlator: make 'xlator_api' mandatory" (Amar Tumballi, 2018-12-06).
  Method = released-source archaeology, not commit-message trust: `git show v5.0:.../xlator.c` has 4
  by-name `dlsym` calls, `v6.0` has 0; the commit diff removes exactly the Translator-101 surface
  (fops/cbks/class_methods/init/fini/reconfigure/notify/dumpops/mem_acct_init). The commit is doubled
  by an upstream history rewrite (`af7e957b49` release line / `67bbd5471a` current devel, identical
  patch-id `571f8c9e16...`), so `git tag --contains 67bbd5471a` misreports v9.0 - the v5.0<->v6.0
  released-code diff is the authoritative answer. v5.0 was a one-release deprecation window (tries
  `xlator_api` first, falls back to by-name). Baked into README sec. 4 + sec. 8.
