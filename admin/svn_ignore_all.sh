#!/bin/sh

# Usage:
#   cd your_svn_working_dir
#   svn_ignore_all.sh

# This script will add all unversioned files (those that are reported
# with an initial '?' by 'svn status') to the svn:ignore property of
# the respective directories. This is helpful if you think adding the
# non-versioned files to svn:ignore manually is too much
# work. However, make sure you have added all files to svn that are
# indeed important, otherwise they will not be reported as unversioned
# anymore. (You can add them to svn anyway, though; only the reporting
# of 'svn status' is concerned by this change.)

# If you are satisfied with the result, of course you need to commit
# your change to svn yourself.

FILES=`svn status | grep '^?' | cut -b8-`

for F in ${FILES} ; do
    DIR=`dirname ${F} `
    FILE=`basename ${F} `
    OLDPROP=`svn propget svn:ignore ${DIR} `
    NEWPROP=`echo -e "${OLDPROP}\n${FILE}\n" | grep -v '^$' | sort `
    echo svn propset svn:ignore "${NEWPROP}" ${DIR}
    svn propset svn:ignore "${NEWPROP}" ${DIR}
done
