#!/bin/sh

# Invoke this remotely on
#   ssh cf-shell.df.net 
# and it will send the email from the host x86-netbsd1 only a
# subset of hosts will actually send emails

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

FROM_EMAIL="cstim@cf.sourceforge.net"
if [ ${PACKAGE} = "gwenhywfar" ] ; then
  TO_EMAIL="gwenhywfar-cvs@lists.sourceforge.net"
else
  TO_EMAIL="cstim@users.sourceforge.net"
fi
SUBJECT="${MODULE} on compile farm: Result summary of automatic test"
TMPFILE=summary.txt

echo "Subject: ${SUBJECT}" > ${TMPFILE}
echo "To: ${TO_EMAIL}" >> ${TMPFILE}
echo "From: ${FROM_EMAIL}" >> ${TMPFILE}
echo "" >> ${TMPFILE}

head -60 resulttext-${PACKAGE}-*.txt >> ${TMPFILE}

if [ "${HOSTNAME}" = "shell.cf.sourceforge.net" ] ; then
    ssh x86-netbsd1 "/usr/sbin/sendmail -i -t -f${FROM_EMAIL} < ${TMPFILE}"
else
    if [ -x /usr/sbin/sendmail ] ; then
	/usr/sbin/sendmail -i -t -f${FROM_EMAIL} < ${TMPFILE}
    else
	mail -s "${SUBJECT}" ${TO_EMAIL} < ${TMPFILE}
    fi
fi
