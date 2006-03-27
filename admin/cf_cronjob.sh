#!/bin/sh

# Invoke this remotely by 
#   ssh cf-shell.df.net "./cf_cronjob.sh [PACKAGE]"
# but it will send the email from yet another host

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

COMMAND="./daily_build.sh ${PACKAGE} ${MODULE}"

USE_HOSTS="amd64-linux1 x86-linux1 x86-linux2 openpower-linux1 sparc-solaris1"
#USE_HOSTS="amd64-linux1"

BATCH_MARK_FILE=batchprogress-${PACKAGE}
touch ${BATCH_MARK_FILE}

for A in ${USE_HOSTS} ; do
  ssh ${A} ${COMMAND}
done

# Send the mail by a separate script
./summary_mail.sh ${PACKAGE} ${MODULE}

rm ${BATCH_MARK_FILE}
