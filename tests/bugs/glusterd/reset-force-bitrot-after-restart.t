#!/bin/bash
#
# "volume reset <vol> force" after a glusterd restart, with bitrot enabled.
#
# The reset walk (dict_foreach over the volume options) used to read every
# removed key after dict_del() had freed it, and to delete features.scrub from
# inside the walk when it removed features.bitrot. After a restart the store
# reloads the options in reverse order, so the scrub pair is the one the walk
# visits right after bitrot: deleting it there left the walk on a freed pair.
# A --enable-asan glusterd dies on either; on any build the reset must remove
# both options and glusterd must keep answering.
#
. $(dirname $0)/../../include.rc
. $(dirname $0)/../../volume.rc

cleanup;

TEST glusterd
TEST pidof glusterd
TEST $CLI volume create $V0 $H0:$B0/${V0}1
TEST $CLI volume start $V0
TEST $CLI volume bitrot $V0 enable
EXPECT 'on' volume_option $V0 features.bitrot
EXPECT 'Active' volume_option $V0 features.scrub

# reload the volume options from the store
TEST killall glusterd
TEST glusterd
TEST pidof glusterd
EXPECT 'Active' volume_option $V0 features.scrub

TEST $CLI volume reset $V0 force
TEST pidof glusterd
EXPECT '' volume_option $V0 features.bitrot
EXPECT '' volume_option $V0 features.scrub

cleanup;
