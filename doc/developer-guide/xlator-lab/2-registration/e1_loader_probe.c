/*
 * e1_loader_probe.c - hermetic reproduction of the loader's registration check.
 *
 * Mirrors libglusterfs xlator_dynload_apis() (xlator.c:262-278):
 *     xlapi = dlsym(handle, "xlator_api");
 *     if (!xlapi) { gf_smsg(... LG_MSG_DLSYM_ERROR ...); ret = -1; goto out; }
 *
 * For each .so given on argv, report whether `xlator_api` is present and what
 * the loader's verdict would therefore be. By-name fops/init are reported too,
 * to show they are present-but-ignored.
 *
 * Build: cc -o e1_loader_probe e1_loader_probe.c -ldl
 * Run:   LD_LIBRARY_PATH=<worktree>/libglusterfs/src/.libs ./e1_loader_probe a.so b.so
 */
#include <dlfcn.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    int i;
    for (i = 1; i < argc; i++) {
        void *h = dlopen(argv[i], RTLD_NOW);
        if (!h) {
            printf("%-16s dlopen FAILED: %s\n", argv[i], dlerror());
            continue;
        }
        void *api = dlsym(h, "xlator_api");
        void *fops = dlsym(h, "fops");
        void *init = dlsym(h, "init");
        printf("%-16s xlator_api=%-7s  by-name[fops=%s init=%s]  => loader verdict: %s\n",
               argv[i],
               api ? "PRESENT" : "ABSENT",
               fops ? "y" : "n",
               init ? "y" : "n",
               api ? "LOAD OK"
                   : "HARD-FAIL (LG_MSG_DLSYM_ERROR, xlator.c:273-278)");
        dlclose(h);
    }
    return 0;
}
