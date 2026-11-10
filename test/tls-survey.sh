#!/bin/sh
# Connects to every HBCI PIN/TAN server in an AqBanking bank data file with
# one or more builds of gwentest, and compares what each build reports about
# the TLS handshake and the server certificate.
#
# This needs network access, so it is not part of "make check"; run it by
# hand. No data is sent to the servers: each certificate is recorded and then
# rejected, which closes the connection.
#
# Usage: tls-survey.sh BANKDATAFILE GWENTEST [GWENTEST...]
#   BANKDATAFILE  e.g. /usr/share/aqbanking/bankinfo/de/banks.data, or
#                 de/banks.data from AqBanking's
#                 src/libs/plugins/bankinfo/generic/de.tar.bz2
#   GWENTEST      a gwentest program, e.g. test/gwentest of a GnuTLS build
#                 and of an OpenSSL build
#
# The record of each build goes to tls-survey-N.txt (N counts from 1). The
# script compares the first record with each other one and exits with 1 if
# they differ. The builds run at the same time, so a server that changes its
# certificate during the survey is unlikely to show up as a difference.

if test $# -lt 2; then
  sed -n '2,/^$/s/^# \{0,1\}//p' "$0" >&2
  exit 2
fi

bankdata="$1"
shift

LC_ALL=C
export LC_ALL
GWEN_LOGLEVEL=error
export GWEN_LOGLEVEL

n=0
pids=""
for gwentest in "$@"; do
  n=$((n+1))
  echo "Survey with $gwentest -> tls-survey-$n.txt" >&2
  "$gwentest" tlssurvey "$bankdata" </dev/null >"tls-survey-$n.txt" &
  pids="$pids $!"
done
for pid in $pids; do
  wait "$pid" || exit 2
done

rv=0
i=2
while test $i -le $n; do
  if ! diff -u tls-survey-1.txt "tls-survey-$i.txt"; then
    rv=1
  fi
  i=$((i+1))
done
exit $rv
