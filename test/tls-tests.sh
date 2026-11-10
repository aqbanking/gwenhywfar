#!/bin/sh
# Runs the local TLS tests ("gwentest tlstest") for "make check". They
# need no network: client and server run on this host over a socketpair.
#
# The record of each scenario goes to stdout and to tls-record.txt.

LC_ALL=C
export LC_ALL
GWEN_LOGLEVEL=error
export GWEN_LOGLEVEL

./gwentest tlstest "${srcdir:-.}/testdata/tls" >tls-record.txt
rv=$?
cat tls-record.txt
exit $rv
