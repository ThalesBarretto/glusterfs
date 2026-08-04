/*
 * legacy_only.c - exports ONLY the pre-xlator_api by-name globals
 * (fops/cbks/init/fini), with NO `xlator_api` symbol.
 *
 * This is what Translator 101 told you to write. The modern loader
 * (xlator_dynload_apis, xlator.c:272) does dlsym(handle,"xlator_api")
 * and hard-fails when it is absent - so this module is refused.
 */
#include <glusterfs/xlator.h>
#include <glusterfs/defaults.h>
#include <glusterfs/logging.h>

static int32_t
lo_lookup(call_frame_t *frame, xlator_t *this, loc_t *loc, dict_t *xdata)
{
    STACK_WIND(frame, default_lookup_cbk, FIRST_CHILD(this),
               FIRST_CHILD(this)->fops->lookup, loc, xdata);
    return 0;
}

int32_t
init(xlator_t *this)
{
    gf_log(this->name, GF_LOG_INFO, "legacy_only init");
    return 0;
}

void
fini(xlator_t *this)
{
    return;
}

struct xlator_fops fops = {
    .lookup = lo_lookup,
};

struct xlator_cbks cbks = {};

/* deliberately NO `xlator_api` symbol - the legacy by-name surface only. */
