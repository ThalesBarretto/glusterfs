/*
 * anatomy_skel.c - a minimal modern GlusterFS xlator.
 *
 * Demonstrates the smallest loadable xlator: the single xlator_api export,
 * init/fini, and ONE real FOP (lookup) that passes through to the child.
 * Every other FOP is back-filled by fill_defaults() at load time.
 *
 * Built standalone (out-of-tree) via the Makefile beside this file - the
 * modern replacement for the stale Translator-101 recipe.
 *
 * Starting draft: compile-and-fix against the glusterfs headers at the anchor.
 */
#include <glusterfs/xlator.h>
#include <glusterfs/defaults.h>
#include <glusterfs/logging.h>

static int32_t
skel_lookup(call_frame_t *frame, xlator_t *this, loc_t *loc, dict_t *xdata)
{
    gf_log(this->name, GF_LOG_INFO, "anatomy-skel: lookup %s",
           (loc && loc->path) ? loc->path : "(null)");

    /* Pass the request down to our single child; let the framework's
     * default callback unwind the reply straight back up. */
    STACK_WIND(frame, default_lookup_cbk, FIRST_CHILD(this),
               FIRST_CHILD(this)->fops->lookup, loc, xdata);
    return 0;
}

int32_t
init(xlator_t *this)
{
    if (!this->children || this->children->next) {
        gf_log(this->name, GF_LOG_ERROR,
               "anatomy-skel needs exactly one child");
        return -1;
    }
    if (!this->parents)
        gf_log(this->name, GF_LOG_WARNING, "dangling volume (no parent)");

    gf_log(this->name, GF_LOG_INFO, "anatomy-skel loaded");
    return 0;
}

void
fini(xlator_t *this)
{
    return;
}

struct xlator_fops fops = {
    .lookup = skel_lookup,
};

struct xlator_cbks cbks = {};

xlator_api_t xlator_api = {
    .init = init,
    .fini = fini,
    .fops = &fops,
    .cbks = &cbks,
    .identifier = "anatomy-skel",
    .category = GF_MAINTAINED,
};
