#!/bin/sh
# Runs one gwentest command as a test of "make installcheck"; see
# Makefile.am.
#
# Usage: gwentest-run.sh GWENTEST DIR/COMMAND.gwt
#
# Environment:
#   GWENTEST_TIMEOUT  time limit in seconds (default 300)
#   GWENTEST_SKIP     space-separated commands to skip
set -u

gwentest=$1
name=$(basename "$2" .gwt)

case " ${GWENTEST_SKIP:-} " in
  *" $name "*)
    echo "$name is in GWENTEST_SKIP"
    exit 77
    ;;
esac

cd "$(dirname "$2")/run" || exit 99
# Perl's alarm gives a time limit on both Linux and macOS, which has no
# timeout(1) by default.
exec perl -e 'alarm shift; exec @ARGV or die "exec: $!\n"' \
  "${GWENTEST_TIMEOUT:-300}" "$gwentest" "$name" </dev/null
