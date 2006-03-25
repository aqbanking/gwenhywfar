#!/bin/sh

# Invoke this remotely by 
#   ssh cf-shell.df.net 'ssh x86-netbsd1 "./cf_cronjob.sh [PACKAGE]"'
# because somehow only a subset of hosts will actually send emails

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

COMMAND='nohup ./daily_build.sh ${PACKAGE} 2>/dev/null >/dev/null &'
#COMMAND='./daily_build.sh ${PACKAGE}'

USE_HOSTS="openpower-linux1 amd64-linux1 sparc-solaris1"
#USE_HOSTS="openpower-linux1"

BATCH_MARK_FILE=batchprogress-${PACKAGE}
touch ${BATCH_MARK_FILE}

for A in ${USE_HOSTS} ; do
  ssh ${A} ${COMMAND}
done

FROM_EMAIL="cstim@cf.sourceforge.net"
if [ ${PACKAGE} = "gwenhywfar" ] ; then
  TO_EMAIL="gwenhywfar-cvs@lists.sourceforge.net"
else
  TO_EMAIL="cstim@users.sourceforge.net"
fi
SUBJECT="${MODULE} on compile farm: Result summary of automatic test"
TMPFILE=summary.txt

cat > ${TMPFILE} <<EOF
Subject: ${SUBJECT}
To: ${TO_EMAIL}
From: ${FROM_EMAIL}

EOF
head -50 resulttext-*.txt >> ${TMPFILE}

if [ -x /usr/sbin/sendmail ] ; then
    /usr/sbin/sendmail -i -t -f${FROM_EMAIL} < ${TMPFILE}
else
    mail -s "${SUBJECT}" ${TO_EMAIL} < ${TMPFILE}
fi

rm ${BATCH_MARK_FILE}
