/*
 * mismatch.c - exercises the STACK_WIND typecheck hack (F8).
 *
 * STACK_WIND_COMMON (stack.h:295) contains:
 *     typeof(fn##_cbk) tmp_cbk = rfn;
 * where `fn` is e.g. child->fops->lookup, so `fn##_cbk` is the lookup_cbk MEMBER
 * of struct xlator_fops. typeof() recovers its type (fop_lookup_cbk_t) and the
 * callback you pass (rfn) is assigned to it - so the compiler verifies your
 * callback's signature matches the FOP. The `_cbk` half of the struct exists
 * solely to make this compile-time check possible.
 *
 * m_good winds lookup with a lookup-shaped callback  -> must compile clean.
 * m_bad  winds lookup with a writev-shaped callback  -> must be diagnosed.
 *
 * Build clean path:  cc <flags> -DGOOD_ONLY -fsyntax-only mismatch.c
 * Build mismatch:    cc <flags>             -fsyntax-only mismatch.c   (expect error)
 */
#include <glusterfs/xlator.h>
#include <glusterfs/defaults.h>

/* Correctly-typed lookup callback (9 params, matches fop_lookup_cbk_t). */
static int32_t
good_lookup_cbk(call_frame_t *frame, void *cookie, xlator_t *this,
                int32_t op_ret, int32_t op_errno, inode_t *inode,
                struct iatt *buf, dict_t *xdata, struct iatt *postparent)
{
    return 0;
}

/* A writev-shaped callback (8 params) - WRONG type for a lookup wind. */
static int32_t
wrong_writev_cbk(call_frame_t *frame, void *cookie, xlator_t *this,
                 int32_t op_ret, int32_t op_errno, struct iatt *prebuf,
                 struct iatt *postbuf, dict_t *xdata)
{
    return 0;
}

int32_t
m_good(call_frame_t *frame, xlator_t *this, loc_t *loc, dict_t *xdata)
{
    STACK_WIND(frame, good_lookup_cbk, FIRST_CHILD(this),
               FIRST_CHILD(this)->fops->lookup, loc, xdata);
    return 0;
}

#ifndef GOOD_ONLY
int32_t
m_bad(call_frame_t *frame, xlator_t *this, loc_t *loc, dict_t *xdata)
{
    /* lookup wind, but a writev-shaped callback: typeof(lookup_cbk) tmp = rfn
       must fail to type-check. */
    STACK_WIND(frame, wrong_writev_cbk, FIRST_CHILD(this),
               FIRST_CHILD(this)->fops->lookup, loc, xdata);
    return 0;
}
#endif
