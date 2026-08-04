# E3 - STACK_WIND typecheck hack. -fsyntax-only (no artifact).
GLFS_SRC ?= $(CURDIR)/../../../..
GFLAGS = -DHAVE_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DGF_LINUX_HOST_OS \
         -include $(GLFS_SRC)/config.h -include $(GLFS_SRC)/site.h \
         -I$(GLFS_SRC) -I$(GLFS_SRC)/libglusterfs/src -I$(GLFS_SRC)/rpc/xdr/src \
         -I/usr/include/uuid -I/usr/include/tirpc

# Must succeed (lookup wind + lookup_cbk):
good:
	$(CC) $(GFLAGS) -DGOOD_ONLY -fsyntax-only mismatch.c && echo "GOOD: compiled clean"

# Must fail (lookup wind + writev_cbk) - the typecheck hack catches it:
bad:
	! $(CC) $(GFLAGS) -fsyntax-only mismatch.c && echo "BAD: correctly rejected"
