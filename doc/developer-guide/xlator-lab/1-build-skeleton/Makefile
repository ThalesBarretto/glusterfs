# Modern standalone xlator build recipe.
# Replaces the stale Translator-101 Makefile (paths/flags no longer valid).
# GLFS_SRC must point at a configured + built glusterfs tree. The default
# resolves to this repo when the lab lives in-tree; override for an external tree.

GLFS_SRC ?= $(CURDIR)/../../../..

TARGET   = anatomy_skel.so
SRC      = anatomy_skel.c

# Flags distilled from the authoritative compile_commands.json entry for template.c.
# LOAD-BEARING and non-obvious (why the classic Translator-101 recipe no longer builds):
#   -include config.h -include site.h : force-included; neither the .c nor the glusterfs
#       headers pull them, yet atomic.h etc. need autoconf values (SIZEOF_LONG, ...).
#   -I rpc/xdr/src, -I/usr/include/{uuid,tirpc} : transitive header deps.
CFLAGS   = -fPIC -Wall -g -DHAVE_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DGF_LINUX_HOST_OS \
           -include $(GLFS_SRC)/config.h -include $(GLFS_SRC)/site.h \
           -I$(GLFS_SRC) -I$(GLFS_SRC)/libglusterfs/src -I$(GLFS_SRC)/rpc/xdr/src \
           -I/usr/include/uuid -I/usr/include/tirpc
LDFLAGS  = -shared -nostartfiles -L$(GLFS_SRC)/libglusterfs/src/.libs -lglusterfs

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)
