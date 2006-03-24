#!/bin/bash

PACKAGE=gwenhywfar
MODULE=${PACKAGE}
LOGFILE=${HOME}/${MODULE}-${HOSTNAME}.log

#FRESH_CHECKOUT=0
FRESH_CHECKOUT=1

SEND_EMAIL=0

echo "### Starting build on:" > ${LOGFILE}
uname -a >> ${LOGFILE}

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

if [ ${FRESH_CHECKOUT} -eq 0 ] ; then
  # or an update of existing module
  cvs -q update > /dev/null 2>&1
  rm -rf autom4te.cache
fi

### Now the actual test compile

echo "### Building build system " >> ${LOGFILE} 2>&1
make -f Makefile.cvs >> ${LOGFILE} 2>&1
build_system_rv=$?

echo "### Configuring " >> ${LOGFILE} 2>&1
./configure --enable-debug --enable-warnings --enable-error-on-warning --prefix=${HOME}/usr >> ${LOGFILE} 2>&1
configure_rv=$?

echo "### make " >> ${LOGFILE} 2>&1
make >> ${LOGFILE} 2>&1
make_rv=$?

if [ ${make_rv} -eq 0 ] ; then
  echo "### make check " >> ${LOGFILE} 2>&1
  make check >> ${LOGFILE} 2>&1
  make_check_rv=$?
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

#echo -e "\n### Summary return values (zero==success):\n Build system : ${build_system_rv}\n configure    : ${configure_rv}\n make         : ${make_rv}\n make check   : ${make_check_rv}\n###"

### Send email
FROM_EMAIL="cstim@cf.sourceforge.net"
#TO_EMAIL="cstim@users.sourceforge.net"
TO_EMAIL="gwenhywfar-cvs@lists.sourceforge.net"
SUBJECT="${MODULE} on ${HOSTNAME}: Results of automatic test"
TMPFILE=resulttext-${HOSTNAME}.txt

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

Full log file follows.

EOF
cat ${LOGFILE} >> ${TMPFILE}

if [ ${SEND_EMAIL} -ne 0 ] ; then
  if [ -x /usr/sbin/sendmail ] ; then
    /usr/sbin/sendmail -i -t -f${FROM_EMAIL} < ${TMPFILE}
  else
    mail -s "${SUBJECT}" ${TO_EMAIL} < ${TMPFILE}
  fi
fi

### and cleanup of log files
rm -f ${LOGFILE}
