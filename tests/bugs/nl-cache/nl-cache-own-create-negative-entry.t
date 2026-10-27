#!/bin/bash

# Without the positive entry cache (the default, and the nl-cache and samba
# groups), nl-cache records a negative entry for every name a lookup does not
# find, but never dropped it when this same client then created the name: the
# brick's cache invalidation does not go back to the client that made the
# change. Once the kernel had forgotten the new inode, the next lookup of the
# name was a name lookup, and nl-cache answered it with ENOENT from its cache
# (up to performance.nl-cache-timeout, 600s in the groups).

. $(dirname $0)/../../include.rc
. $(dirname $0)/../../volume.rc

cleanup;

# the two loops below run their TEST 6 times each: 10 more than the lines
TESTS_EXPECTED_IN_LOOP=10

TEST glusterd
TEST $CLI volume create $V0 $H0:$B0/${V0}{1,2,3}
TEST $CLI volume set $V0 group nl-cache
TEST $CLI volume start $V0

TEST $GFS --volfile-id=/$V0 --volfile-server=$H0 $M0

# Every name below is looked up while missing, so nl-cache caches it as
# negative, and then created by this client.
for n in file fifo dir symlink hardlink renamed; do
        TEST ! stat $M0/$n
done
TEST touch $M0/file
TEST mkfifo $M0/fifo
TEST mkdir $M0/dir
TEST ln -s file $M0/symlink
TEST ln $M0/file $M0/hardlink
TEST touch $M0/tmp
TEST mv $M0/tmp $M0/renamed

# Make the kernel forget the inodes: the next access of each name is a lookup
# by name, which must find it.
drop_cache $M0
for n in file fifo dir symlink hardlink renamed; do
        TEST stat $M0/$n
done

cleanup;
