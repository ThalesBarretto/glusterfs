# E1 - registration experiment. Builds the legacy-only xlator (no xlator_api)
# and the hermetic loader probe.
GLFS_SRC ?= $(CURDIR)/../../../..

GFLAGS  = -fPIC -Wall -g -DHAVE_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DGF_LINUX_HOST_OS \
          -include $(GLFS_SRC)/config.h -include $(GLFS_SRC)/site.h \
          -I$(GLFS_SRC) -I$(GLFS_SRC)/libglusterfs/src -I$(GLFS_SRC)/rpc/xdr/src \
          -I/usr/include/uuid -I/usr/include/tirpc
GLDFLAGS = -shared -nostartfiles -L$(GLFS_SRC)/libglusterfs/src/.libs -lglusterfs

all: legacy_only.so e1_loader_probe

legacy_only.so: legacy_only.c
	$(CC) $(GFLAGS) legacy_only.c $(GLDFLAGS) -o legacy_only.so

e1_loader_probe: e1_loader_probe.c
	$(CC) -Wall -g e1_loader_probe.c -ldl -o e1_loader_probe

clean:
	rm -f legacy_only.so e1_loader_probe
