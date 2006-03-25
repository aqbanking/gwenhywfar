#!/bin/bash

if [ $# -lt 1 ] ; then
  PACKAGE=gwenhywfar
else
  PACKAGE=$1
fi
if [ $# -lt 2 ] ; then
  MODULE=${PACKAGE}
else
  MODULE=$2
fi
LOGFILE=${HOME}/${MODULE}-${HOSTNAME}.log
INSTALLPREFIX=${HOME}/usr-${HOSTNAME}

#FRESH_CHECKOUT=0
FRESH_CHECKOUT=1

# Only send email if this hasn't been called from the batch processing
# script
BATCH_MARK_FILE=batchprogress-${PACKAGE}
if [ -f ${BATCH_MARK_FILE} -o ${HOSTNAME} = "amd64-linux1.sourceforge.net" ] ; then
  SEND_EMAIL=0
else
  SEND_EMAIL=1
fi

echo "### Starting build on:" > ${LOGFILE}
uname -a >> ${LOGFILE}

# The actual module build. Has been moved to a function to make it
# easier to build several modules in a row.
#   usage: module_build package_name module_name
function module_build()
{
    local PACKAGE="$1"
    local MODULE="$2"

    ### Get the source code
    echo "### CVS Checkout: " >> ${LOGFILE} 2>&1
    ###cvs -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/gwenhywfar login

    # Either a fresh checkout
    if [ ${FRESH_CHECKOUT} -ne 0 ] ; then
      MODULEDIR=${MODULE}-${HOSTNAME}
      cvs -q -z3 -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/${PACKAGE} co -P -d${MODULEDIR} ${MODULE} > /dev/null 2>&1
    else
      MODULEDIR=${MODULE}
    fi

    cd ${MODULEDIR}

    # or an update of existing module
    if [ ${FRESH_CHECKOUT} -eq 0 ] ; then
      cvs -q update > /dev/null 2>&1
      rm -rf autom4te.cache
    fi

    ### Now the actual test compile

    echo "### Building build system " >> ${LOGFILE} 2>&1
    ACLOCAL_FLAGS="-I ${INSTALLPREFIX}/share/aclocal"
    if [ -f Makefile.cvs ] ; then
      make -f Makefile.cvs >> ${LOGFILE} 2>&1
    elif [ -f Makefile.dist ] ; then
      make -f Makefile.dist >> ${LOGFILE} 2>&1
    elif [ -f autogen.sh ] ; then
      ./autogen.sh >> ${LOGFILE} 2>&1
    else
      echo "## Oops, no method for build system detected!" >> ${LOGFILE} 2>&1
    fi
    build_system_rv=$?

    echo "### Configuring " >> ${LOGFILE} 2>&1
    if [ ${PACKAGE} = "aqbanking" ] ; then
	./configure --enable-debug --enable-warnings --enable-error-on-warning --prefix=${INSTALLPREFIX} --with-gwen-dir=${INSTALLPREFIX} --with-frontends="cbanking" --with-backends="aqdtaus aqnone aqhbci" >> ${LOGFILE} 2>&1
    else
	./configure --enable-debug --enable-warnings --enable-error-on-warning --prefix=${INSTALLPREFIX} >> ${LOGFILE} 2>&1
    fi
    configure_rv=$?

    echo "### make " >> ${LOGFILE} 2>&1
    make >> ${LOGFILE} 2>&1
    make_rv=$?

    if [ ${make_rv} -eq 0 ] ; then
      echo "### make check " >> ${LOGFILE} 2>&1
      make check >> ${LOGFILE} 2>&1
      make_check_rv=$?

      if [ ${PACKAGE} = "gwenhywfar" ] ; then
        echo "### make install " >> ${LOGFILE} 2>&1
        make install >> ${LOGFILE} 2>&1
      fi
    else
      echo "### Skipping make check " >> ${LOGFILE} 2>&1
      make_check_rv="skipped"
    fi

    ## Store version string
    if [ -f Makefile ] ; then
      version_string=`grep '^VERSION' Makefile `
    else
      version_string="no Makefile found"
    fi

    cd ..

    ### Finished

    echo "### Cleanup " >> ${LOGFILE} 2>&1
    if [ ${FRESH_CHECKOUT} -ne 0 ] ; then
      rm -rf ${MODULEDIR} >> ${LOGFILE} 2>&1
    fi
}
# end of function module_build()

###
# Build only one module. Might be changed later to build several
# modules in a row, needed for e. g. the combinations
# gwenhywfar/aqbanking.

if [ ${PACKAGE} = "aqbanking" ] ; then
  module_build gwenhywfar gwenhywfar
  module_build ${PACKAGE} ${MODULE}
else
  module_build ${PACKAGE} ${MODULE}
fi

#echo -e "\n### Summary return values (zero==success):\n Build system : ${build_system_rv}\n configure    : ${configure_rv}\n make         : ${make_rv}\n make check   : ${make_check_rv}\n###"

### Send email
FROM_EMAIL="cstim@cf.sourceforge.net"
if [ ${PACKAGE} = "gwenhywfar" ] ; then
  TO_EMAIL="gwenhywfar-cvs@lists.sourceforge.net"
else
  TO_EMAIL="cstim@users.sourceforge.net"
fi
SUBJECT="${MODULE} on ${HOSTNAME}: Results of automatic test"
TMPFILE=resulttext-${MODULE}-${HOSTNAME}.txt

cat > ${TMPFILE} <<EOF
Subject: ${SUBJECT}
To: ${TO_EMAIL}
From: ${FROM_EMAIL}

Results of test suite on ${HOSTNAME} 
for ${MODULE} version ${version_string}:

Summary return values (zero==success):
  Build system : ${build_system_rv}
  configure    : ${configure_rv}
  make         : ${make_rv}
  make check   : ${make_check_rv}

Last 40 lines of log file follows.

EOF
#cat ${LOGFILE} >> ${TMPFILE}
tail -40 ${LOGFILE} >> ${TMPFILE}

if [ ${SEND_EMAIL} -ne 0 ] ; then
  if [ -x /usr/sbin/sendmail ] ; then
    /usr/sbin/sendmail -i -t -f${FROM_EMAIL} < ${TMPFILE}
  else
    mail -s "${SUBJECT}" ${TO_EMAIL} < ${TMPFILE}
  fi
fi

### and cleanup of log files and installation prefix
rm -f ${LOGFILE}
rm -rf ${INSTALLPREFIX}
