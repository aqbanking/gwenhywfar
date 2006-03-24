#!/bin/sh

#COMMAND='nohup ./daily_build.sh 2>/dev/null >/dev/null &'
COMMAND='./daily_build.sh'

#USE_HOSTS="openpower-linux1 amd-linux1 sparc-solaris1 x86-netbsd1"
USE_HOSTS="openpower-linux1"

for A in ${USE_HOSTS} ; do
  ssh ${A} ${COMMAND}
done

FROM_EMAIL="cstim@cf.sourceforge.net"
TO_EMAIL="cstim@users.sourceforge.net"
#TO_EMAIL="gwenhywfar-cvs@lists.sourceforge.net"
SUBJECT="${MODULE} on compile farm: Result summary of automatic test"
TMPFILE=summary.txt

cat > ${TMPFILE} <<EOF
Subject: ${SUBJECT}
To: ${TO_EMAIL}
From: ${FROM_EMAIL}

EOF
head -13 resulttext-*.txt >> ${TMPFILE}

if [ -x /usr/sbin/sendmail ] ; then
    /usr/sbin/sendmail -i -t -f${FROM_EMAIL} < ${TMPFILE}
else
    mail -s "${SUBJECT}" ${TO_EMAIL} < ${TMPFILE}
fi
